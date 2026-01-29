// Project
#include "KM1X.h"

KM1X_PRINTER::KM1X_PRINTER(QObject *parent) : BasePrinterDevices(parent)
{
}

bool KM1X_PRINTER::OpenPrinterPort()
{
    this->openPort();

    return is_open;
}

bool KM1X_PRINTER::openPort()
{
    if (devicesCreated)
    {
        // Если девайс для работы с портом обявлен
        is_open = false;

        // Даем девайсу название порта
        serialPort->setPortName(comName);

        if (serialPort->open(QIODevice::ReadWrite))
        {
            // Устанавливаем параметры открытия порта
            is_open = false;

            if (!serialPort->setDataBits(QSerialPort::Data8))
                return false;
            if (!serialPort->setParity(QSerialPort::NoParity))
                return false;
            if (!serialPort->setStopBits(QSerialPort::OneStop))
                return false;
            if (!serialPort->setFlowControl(QSerialPort::NoFlowControl))
                return false;
            if (!serialPort->setBaudRate(QSerialPort::Baud19200))
                return false;

            if (port_speed.toInt() == 0)
            {
                if (!serialPort->setBaudRate(QSerialPort::Baud115200))
                    return false;
            }
            else if (port_speed == "1")
            {
                if (!serialPort->setBaudRate(QSerialPort::Baud9600))
                    return false;
            }
            else if (port_speed == "2")
            {
                //                if (!serialPort->setBaudRate(QSerialPort::Baud14400))
                //                return false;
            }
            else if (port_speed == "3")
            {
                if (!serialPort->setBaudRate(QSerialPort::Baud19200))
                    return false;
            }
            else if (port_speed == "4")
            {
                if (!serialPort->setBaudRate(QSerialPort::Baud38400))
                    return false;
            }
            else if (port_speed == "5")
            {
                if (!serialPort->setBaudRate(QSerialPort::Baud57600))
                    return false;
            }
            else if (port_speed == "6")
            {
                if (!serialPort->setBaudRate(QSerialPort::Baud115200))
                    return false;
            } /*else if(port_speed == "7"){
                 if (!serialPort->setBaudRate(QSerialPort::UnknownBaud)) return false;
             }*/

            is_open = true;
        }
        else
        {
            is_open = false;
        }
    }
    else
    {
        is_open = false;
    }

    return is_open;
}

bool KM1X_PRINTER::isEnabled()
{
    return true;
}

bool KM1X_PRINTER::isItYou()
{
    return true;
}

bool KM1X_PRINTER::printCheck(const QString &aCheck)
{
    QByteArray cmd;
    QByteArray answer;
    bool respData = false;

    // Проинициализируем принтер
    cmd.push_back(CMDKM1X::PrinterCommandFirstByte);
    cmd.push_back(0x74);
    cmd.push_back(0x3b);

    cmd.append(encodingString(aCheck, CScodec::c_IBM866));

    cmd.push_back(0x0a);
    cmd.push_back(0x0d);
    cmd.push_back(0x1d);
    cmd.push_back(0x56);
    cmd.push_back(0x42);
    cmd.push_back('\0');

    if (!this->sendCommand(cmd, false, 0, respData, answer, 0))
        return false;

    return true;
}

void KM1X_PRINTER::print(const QString &aCheck)
{
    // Печатаем текст
    printCheck(aCheck);
}
