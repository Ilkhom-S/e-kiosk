
#include "CitizenCBM1000.h"

CitizenCBM1000_PRINTER::CitizenCBM1000_PRINTER(QObject *parent) : BasePrinterDevices(parent) {
    //    printer_name = "Custom-VKP80";
}

bool CitizenCBM1000_PRINTER::OpenPrinterPort() {
    this->openPort();
    return is_open;
}

bool CitizenCBM1000_PRINTER::openPort() {
    if (devicesCreated) {
        // Если девайс для работы с портом обявлен
        is_open = false;
        // Даем девайсу название порта
        // if(Debugger)  qDebug() << "devicePort->setDeviceName(com_Name); - " <<
        // com_Name;
        serialPort->setPortName(com_Name);

        if (serialPort->open(QIODevice::ReadWrite)) {
            // Если Девайсу удалось открыть порт

            // Устанавливаем параметры открытия порта
            is_open = false;

            if (!serialPort->setDataBits(QSerialPort::Data8)) {
                return false;
            }
            //                if (!devicePort->setParity(AbstractSerial::ParityNone))
            //                return false; if
            //                (!devicePort->setStopBits(AbstractSerial::StopBits1))
            //                return false; if
            //                (!devicePort->setFlowControl(AbstractSerial::FlowControlUndefined))
            //                return false; if
            //                (!serialPort->setCharIntervalTimeout(CMDCitizenCBM1000::charTimeOut))
            //                return false;
            if (!serialPort->setBaudRate(QSerialPort::Baud19200)) {
                return false;
            }

            // if(Debugger) qDebug() << "\nPrinter " << CMDCitizenCBM1000::DeviceName
            // << " to Port " << devicePort->deviceName() << " open in " <<
            // devicePort->openMode();

            // if(Debugger) qDebug() << "\n= Defaults parameters =";
            // if(Debugger) qDebug() << "Device name            : " <<
            // devicePort->deviceName(); if(Debugger) qDebug() << "Baud rate : " <<
            // devicePort->baudRate(); if(Debugger) qDebug() << "Data bits : " <<
            // devicePort->dataBits(); if(Debugger) qDebug() << "Parity : " <<
            // devicePort->parity(); if(Debugger) qDebug() << "Stop bits              :
            // " << devicePort->stopBits(); if(Debugger) qDebug() << "Flow : " <<
            // devicePort->flowControl(); if(Debugger) qDebug() << "Char timeout, msec
            // : " << devicePort->charIntervalTimeout();

            is_open = true;
        } else {
            is_open = false;
            //            statusDevices = this->PortError;
            // if(Debugger) qDebug() << "Error opened serial device " <<
            // devicePort->deviceName();
        }
    } else {
        is_open = false;
        // if(Debugger) qDebug() << "Error create serial device " <<
        // devicePort->deviceName();
        //        statusDevices = this->DeviceError;
    }

    return is_open;
}

bool CitizenCBM1000_PRINTER::isItYou() {
    int status = 0;
    bool result = isEnabled(status);
    this->closePort();
    return result;
}

bool CitizenCBM1000_PRINTER::isEnabled(int status) {
    //        int status = 0;
    if (!getStatus(status)) {
        return false;
    }
    return (status != PrinterState::PrinterNotAvailable);
}

bool CitizenCBM1000_PRINTER::getStatus(int &aStatus) {
    aStatus = PrinterState::PrinterNotAvailable;
    // засылаем в порт команду самоидентификации
    QByteArray cmd;
    QByteArray answer;
    bool respData = false;

    cmd.push_back(CMDCitizenCBM1000::PrinterStatusCommandFirstByte);
    cmd.push_back(CMDCitizenCBM1000::PrinterStatusCommandSecondByte);
    if (!this->sendCommand(cmd, true, 200, respData, answer, 0)) {
        // if(Debugger) qDebug() << "AV268::getStatus(): error in
        // sendPacketInPort()";
        return false;
    }

    if (answer.size() < 1) {
        // if(Debugger) qDebug() << QString("AV268::getStatus(): wrong size of
        // buffer. Buffer is: %1").arg(answer.data());
        return false;
    }

    uchar status = 0;
    // В некоторых случаях присылается больше 1 байта
    // тогда наш байт - второй
    if (answer.size() > 1) {
        status = answer[1];
    } else {
        status = answer[0];
    }
    // Проверим, что это наш статус
    if (((status & 0x10) != 0) || ((status & 0x80) != 0)) {
        // Не наш принтер
        // if(Debugger) qDebug() << QString("AV268::getStatus(): wrong byte returned:
        // %1").arg(status);
        return false;
    }
    // Наш принтер
    aStatus = PrinterState::PrinterOK;
    if (status != CMDCitizenCBM1000::PrinterNormalState) {
        // Error
        int code = status & CMDCitizenCBM1000::PrinterTemperatureError;
        if (code > 0) {
            // Temperature error
            aStatus |= PrinterState::TemperatureError;
            // if(Debugger) qDebug() << "AV268::getStatus(): Temperature error";
        }
        code = status & CMDCitizenCBM1000::PrinterNoPaperError;
        if (code > 0) {
            // No paper
            aStatus |= PrinterState::PaperEnd;
            // if(Debugger) qDebug() << "AV268::getStatus(): No paper";
        }
        code = status & CMDCitizenCBM1000::PrinterHeadOpenError;
        if (code > 0) {
            // Printing head open
            aStatus |= PrinterState::PrintingHeadError;
            // if(Debugger) qDebug() << "AV268::getStatus(): Printing head open";
        }
        code = status & CMDCitizenCBM1000::PrinterSystem_Error;
        if (code > 0) {
            // System error
            aStatus |= PrinterState::PrinterError;
            // if(Debugger) qDebug() << "AV268::getStatus(): System error";
        }
        code = status & CMDCitizenCBM1000::PrinterDataReceiveError;
        if (code > 0) {
            // Data receive error
            aStatus |= PrinterState::PortError;
            // if(Debugger) qDebug() << "AV268::getStatus(): Data receive error";
        }
    }
    return true;
}

bool CitizenCBM1000_PRINTER::initialize() {
    // засылаем в порт команду самоидентификации
    QByteArray cmd;
    cmd.push_back(CMDCitizenCBM1000::PrinterStatusCommandFirstByte);
    cmd.push_back(CMDCitizenCBM1000::PrinterInitCommandSecondByte);
    bool respData = false;
    QByteArray response;

    bool res = this->sendCommand(cmd, true, 200, respData, response, 290);

    return res;
}

bool CitizenCBM1000_PRINTER::cut() {
    QByteArray cmd;

    cmd.push_back(CMDCitizenCBM1000::PrinterStatusCommandFirstByte);
    cmd.push_back(CMDCitizenCBM1000::PrinterCutCommandSecondByte);

    bool respData = false;
    QByteArray response;
    bool res = this->sendCommand(cmd, true, 200, respData, response, 50);

    return res;
}

void CitizenCBM1000_PRINTER::getSpecialCharacters(QByteArray &printText) {
    // Устанавливаем если есть жирный фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeBold +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeBold + CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    // Устанавливаем если есть двойной высоты фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                              CScharsetParam::FontTypeDoubleHeight +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeDoubleHeight +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    // Устанавливаем если есть двойной ширины фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter +
                              CScharsetParam::FontTypeDoubleWidth +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeDoubleWidth +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    // Устанавливаем если есть курсивный фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeItalic +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeItalic + CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    // Устанавливаем если есть подчеркнутый фонт
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::FontTypeUnderLine +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::CloseTagSymbol +
                              CScharsetParam::FontTypeUnderLine + CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      asciiNull());

    // Если надо добавить проабел
    QByteArray probel;
    for (int i = 1; i <= leftMargin; i++) {
        probel.append(ASCII::Space);
    }
    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::SpaceCount +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      probel);

    // Добавляем звезды
    int colZ = (checkWidth - 11 - leftMargin) / 2;
    QByteArray star;
    for (int j = 1; j <= colZ; j++) {
        star.append("*");
    }

    printText.replace(QString(CScharsetParam::OpenTagDelimiter + CScharsetParam::StarCount +
                              CScharsetParam::CloseTagDelimiter)
                          .toUtf8(),
                      star);
}

bool CitizenCBM1000_PRINTER::printCheck(const QString &aCheck) {
    // Меняем кодировку
    QByteArray printText;
    printText = this->encodingString(aCheck, CScodec::c_IBM866);

    // Вставляем если есть Подчеркнутый, Жирный, Курсивный... Шрифт
    this->getSpecialCharacters(printText);

    //    QByteArray cmd;
    QByteArray answer;
    bool respData = false;

    return this->sendCommand(printText, false, 0, respData, answer, 0);
}

void CitizenCBM1000_PRINTER::print(const QString &aCheck) {

    // установим размер шрифта
    QByteArray cmd;
    cmd.push_back(CMDCitizenCBM1000::PrinterStatusCommandFirstByte);
    cmd.push_back(0x4D);
    cmd.push_back(0x01);

    bool respData = false;
    QByteArray response;

    this->sendCommand(cmd, false, 200, respData, response, 290);

    // Печатаем текст
    this->printCheck(aCheck);

    feed(3);

    // Обрезка
    this->cut();
}

bool CitizenCBM1000_PRINTER::feed(int aCount) {
    QByteArray cmd;
    bool respData = false;
    QByteArray answer;
    cmd.push_back(0x0A);
    cmd.push_back(0x0D);
    for (int i = 0; i < aCount; ++i) {
        if (!this->sendCommand(cmd, true, 50, respData, answer, 0)) {
            return false;
        }
    }

    return true;
}
