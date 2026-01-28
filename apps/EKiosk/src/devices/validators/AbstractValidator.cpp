// Project
#include "AbstractValidator.h"

bool debugger = false;

BaseValidatorDevices::BaseValidatorDevices(QObject *parent) : QThread(parent)
{
    debugger = false;
    devicesCreated = false;

    this->createDevicePort();
}

bool BaseValidatorDevices::createDevicePort()
{
    serialPort = new QSerialPort(this);

    if (serialPort)
    {
        devicesCreated = true;
    }
    else
    {
        devicesCreated = false;
    }
    return devicesCreated;
}

bool BaseValidatorDevices::isOpened()
{
    if (serialPort->isOpen())
        is_open = true;
    else
        is_open = false;

    return is_open;
}

void BaseValidatorDevices::setPortName(const QString com_Name)
{
    comName = com_Name;
}

void BaseValidatorDevices::setPartNumber(const QString partNumber)
{

    part_number = partNumber;
}

bool BaseValidatorDevices::closePort()
{
    // if(Debugger) qDebug() << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&";
    //    qDebug() << "PORT VALIDATOR CLOSE";
    // if(Debugger) qDebug() << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&";

    if (!isOpened())
    {
        return true;
    }

    //    serialPort->reset();
    serialPort->flush();
    serialPort->close();
    is_open = false;
    return true;
}
void BaseValidatorDevices::printDataToHex(const QByteArray &data)
{
    QByteArray baTmp;
    baTmp.clear();
#if QT_VERSION >= 0x040300
    baTmp = (data.toHex()).toUpper();
#else
    quint8 n = 0;
    for (int i = 0; i < data.size(); i++)
    {
        n = data.at(i);
        if ((n >= 0) && (n <= 15))
            baTmp.append(QByteArray::number(0, 16).toUpper());
        baTmp.append(QByteArray::number(n, 16).toUpper());
        z
    }
#endif
    for (int i = 0; i < baTmp.size(); i += 2)
    {
        // if(Debugger) qDebug() << "[" << baTmp.at(i) << baTmp.at(i + 1) << "]";
    }
}

bool BaseValidatorDevices::sendCommand(QByteArray dataRequest, bool getResponse, int timeResponse,
                                       QByteArray &dataResponse, int timeSleep, bool readAll)
{
    bool respOk = false;

    if (this->isOpened())
    {
        // Если девайс открыт
        respOk = false;

        serialPort->write(dataRequest);
        // if(Debugger) qDebug() << QString("\n --> Request : to port -
        // %1\n").arg(comName);
        //         this->printDataToHex(dataRequest);

        // Задержка после команды
        if (timeSleep > 0)
            this->msleep(timeSleep);

        if (getResponse)
        {
            // Если нам нужен респонс
            bool ret = serialPort->waitForReadyRead(timeResponse);
            if (ret)
            {
                // Есть ответ
                qint64 inByte = serialPort->bytesAvailable();

                if (readAll)
                {
                    dataResponse = serialPort->readAll();
                }
                else
                {
                    dataResponse = serialPort->read(inByte);
                }

                // if(Debugger) qDebug() << QString("\n <-- Response <----\n");
                //                  this->printDataToHex(dataResponse);
                respOk = true;
            }
            else
            {
                respOk = false;
            }
        }

        return respOk;
    }

    return respOk;
}

QString BaseValidatorDevices::cmdName(ValidatorCommands::Enum cmd)
{
    return QVariant::fromValue(cmd).value<QString>();
}
