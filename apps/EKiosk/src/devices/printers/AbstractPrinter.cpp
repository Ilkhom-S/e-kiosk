#include "AbstractPrinter.h"

#include <QtCore/QDebug>
#include <QtCore5Compat/QTextCodec>

BasePrinterDevices::BasePrinterDevices(QObject *parent) : QThread(parent) {
    Debugger = false;
    smallCheck = false;
    viewLogoImg = false;
    devicesCreated = false;
    counterIndicate = false;

    createDevicePort();
}

bool BasePrinterDevices::createDevicePort() {
    serialPort = new QSerialPort(this);

    if (serialPort) {
        // if(Debugger) qDebug() << "Create devicePort is fail!";
        devicesCreated = true;
    } else {
        devicesCreated = false;
    }

    return devicesCreated;
}

bool BasePrinterDevices::isOpened() {
    if (serialPort->isOpen())
        is_open = true;
    else
        is_open = false;

    return is_open;
}

bool BasePrinterDevices::closePort() {
    //    devicePort->flush();
    //    devicePort->reset();
    // if(Debugger) qDebug() << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&";
    // if(Debugger) qDebug() << "PORT PRINTER CLOSE";
    // if(Debugger) qDebug() << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&";

    if (!isOpened()) {
        return true;
    }

    serialPort->close();
    is_open = false;
    return true;
}

QByteArray BasePrinterDevices::packetImage(const QString &aPixelString, uchar aWidth) {
    QByteArray result;
    uchar height = 0;
    if (aWidth)
        height = (uchar)(aPixelString.size() / aWidth);
    uchar horizontalSize = aWidth;   // Размер по горизонтали, который мы передадим принтеру
    uchar verticalSize = height / 8; // Размер по вертикали, который мы передадим принтеру
    if (height % 8) {
        verticalSize++;
    }
    // Разделим каждый столбец на элементы высотой по 8 точек и будем передавать
    // эти элементы в виде 1 байта Это нужно, т.к. каждый байт, переданный
    // принтеру рассматривается как битовая маска из 8 точек и рисунок может
    // получиться в 8 раз больше по высоте
    for (int j = 0; j < aWidth; ++j) {
        int num = 7;           // Степень, в которую будем возводить двойку, чтобы сложить все
                               // биты для байта
        uchar sum = 0;         // Число, объединяющее 8 точек по вертикали
        bool fNeedAdd = false; // Флаг нужен для того, чтобы добавить элементы,
                               // которые являются остатком от деления
        // высоты рисунка на 8
        for (int i = 0; i < height; ++i) {
            fNeedAdd = true;
            int pixel = aPixelString[j * height + i].toLatin1();
            if (pixel > 0) {
                sum = sum + (uchar)pow((double)2, (double)num); // Возведение в степень
            }
            num--;
            if (num < 0) {
                num = 7;
                result.push_back(sum);
                sum = 0;
                fNeedAdd = false;
            }
        }
        if (fNeedAdd)
            result.push_back(sum);
    }
    uchar nil = 0; // Нужно, чтобы запихнуть в массив нули
    int nilsCount = horizontalSize * verticalSize * 8 - (result.size());
    // Количество элементов, которые нужно заполнить нулями
    for (int i = 0; i < nilsCount; ++i) {
        result.push_back(nil);
    }
    return result;
}

void BasePrinterDevices::setPortName(const QString com_Name) {
    com_Name = com_Name;
}

// bool BasePrinterDevices::print(const QString& aCheck)
//{

//}

// void BasePrinterDevices::dispense()
//{

//}

void BasePrinterDevices::setFirm_Pattern(const QString firm_name) {
    company_name = firm_name;
    viewLogoImg = false;
}

void BasePrinterDevices::setSmallBetweenString(bool beetwen) {
    SmallBetweenString = beetwen;
}

void BasePrinterDevices::setCheckSmall(bool smallChek) {
    qDebug() << QString("BasePrinterDevices::setChekSmall - small - %1").arg(smallChek);
    this->smallCheck = smallChek;
    qDebug() << QString("BasePrinterDevices::setChekSmall - smallChek - %1").arg(smallChek);
}

void BasePrinterDevices::setCheckWidth(int width) {
    checkWidth = width;
}

void BasePrinterDevices::setLeftMargin(int left_margin) {
    leftMargin = left_margin;
}

bool BasePrinterDevices::sendCommand(QByteArray dataRequest,
                                     bool getResponse,
                                     int timeResponse,
                                     bool &respOk,
                                     QByteArray &dataResponse,
                                     int timeSleep) {
    if (this->isOpened()) {
        // Если девайс открыт
        respOk = false;
        //        devicePort->flush();
        serialPort->write(dataRequest);
        // if(Debugger) qDebug() << QString("\n --> Request : to port -
        // %1\n").arg(com_Name);
        this->printDataToHex(dataRequest);

        if (getResponse) {
            // Если нам нужен респонс
            this->msleep(timeResponse);
            bool ret = serialPort->waitForReadyRead(timeResponse);
            if (ret) {
                // Есть ответ

                qint64 inByte = serialPort->bytesAvailable();
                dataResponse = serialPort->read(inByte);
                //                 dataResponse.append( devicePort->readAll());
                // if(Debugger) qDebug() << QString("\n <-- Response <----\n");
                this->printDataToHex(dataResponse);
                respOk = true;
            } else {
                respOk = false;
            }
        }

        // Задержка после команды
        this->msleep(timeSleep);
        return true;
    }
    return false;
}

// bool BasePrinterDevices::sendCommand(QByteArray dataRequest, bool
// getResponse, int timeResponse, bool& respOk, QByteArray& dataResponse, int
// timeSleep)
//{
//     if(this->isOpened()){
//     //Если девайс открыт
//         respOk = false;

//        devicePort->write(dataRequest);
//        //if(Debugger) qDebug() << QString("\n --> Request : to port -
//        %1\n").arg(com_Name); this->printDataToHex(dataRequest);

//        if(getResponse){
//        //Если нам нужен респонс
////            bool ret = devicePort->waitForReadyRead(timeResponse);
//            this->msleep(timeResponse);
//            qint64 inByte = devicePort->bytesAvailable();
//            if (inByte){
//            //Есть ответ
//                dataResponse = devicePort->read(inByte);
////                 dataResponse.append( devicePort->readAll());
//                 //if(Debugger) qDebug() << QString("\n <-- Response <----\n");
//                 this->printDataToHex(dataResponse);
//                 respOk = true;
//             }else{
//                respOk = false;
//             }
//        }

//        //Задержка после команды
//        this->msleep(timeSleep);
//        return true;
//    }
//    return false;
//}

QByteArray BasePrinterDevices::encodingString(const QString &text, const QByteArray charCode) {
    // Меняем кодировку
    QByteArray encodedString = text.toUtf8();

    QTextCodec *codecUTF = QTextCodec::codecForName("UTF-8");
    QString string = codecUTF->toUnicode(encodedString);

    QTextCodec *codec = QTextCodec::codecForName(charCode);

    return codec->fromUnicode(string);
}

void BasePrinterDevices::printDataToHex(const QByteArray &data) {

    QByteArray baTmp;
    baTmp.clear();
#if QT_VERSION >= 0x040300
    baTmp = (data.toHex()).toUpper();
#else
    quint8 n = 0;
    for (int i = 0; i < data.size(); i++) {
        n = data.at(i);
        if ((n >= 0) && (n <= 15))
            baTmp.append(QByteArray::number(0, 16).toUpper());
        baTmp.append(QByteArray::number(n, 16).toUpper());
    }
#endif

    for (int i = 0; i < baTmp.size(); i += 2) {
        // if(Debugger) qDebug() << "[" << baTmp.at(i) << baTmp.at(i + 1) << "]";
    }
}

QByteArray BasePrinterDevices::asciiNull() {
    QByteArray data;
    data.append(ASCII::NUL);

    return data;
}

//--------------------------------------------------------------------------------
bool getBit(char aValue, int aShift) {
    return (aValue >> aShift) & 1;
}

//--------------------------------------------------------------------------------

bool positiveMasking(char aValue, char aMask) {
    bool result = true;

    for (int i = 0; i < aValue * 8; ++i) {
        char localMask = 1 << i;

        if (aMask & localMask) {
            result &= getBit(aValue, i);
        }
    }

    return result;
}

//--------------------------------------------------------------------------------

bool negativeMasking(char aValue, char aMask) {
    bool result = true;

    for (int i = 0; i < aValue * 8; ++i) {
        char localMask = 1 << i;

        if (~aMask & localMask) {
            result &= !getBit(aValue, i);
        }
    }

    return result;
}
