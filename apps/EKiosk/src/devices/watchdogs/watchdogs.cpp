#include "watchdogs.h"

#include <QtCore/QDebug>

QStringList WD_List;

WatchDogs::WatchDogs(QObject *parent) : QThread(parent) {
    WD_List << COSMP1::DeviceName;

    createDevicePort();
}

bool WatchDogs::createDevicePort() {

    serialPort = new QSerialPort(this);

    if (!serialPort) {
        devicesCreated = false;
        return false;
    }

    devicesCreated = true;
    return true;
}

bool WatchDogs::closePort() {
    if (!isOpened()) {
        return true;
    }

    serialPort->close();
    is_open = false;
    return true;
}

void WatchDogs::setPort(const QString comName) {
    this->com_Name = comName;
}

bool WatchDogs::isOpened() {
    is_open = serialPort->isOpen();

    return is_open;
}

void WatchDogs::printDataToHex(const QByteArray &data) {

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
        qDebug() << "[" << baTmp.at(i) << baTmp.at(i + 1) << "]";
    }
}

bool WatchDogs::sendCommand(QByteArray dataRequest,
                            bool getResponse,
                            int timeResponse,
                            bool &respOk,
                            QByteArray &dataResponse,
                            int timeSleep) {
    if (this->isOpened()) {
        // Если девайс открыт
        respOk = false;

        serialPort->write(dataRequest);
        qDebug() << QString("\n --> Request : to port - %1\n").arg(com_Name);
        this->printDataToHex(dataRequest);

        if (getResponse) {
            // Если нам нужен респонс
            this->msleep(timeResponse);
            bool ret = serialPort->waitForReadyRead(timeResponse);
            if (ret) {
                // Есть ответ
                qint64 inByte = serialPort->bytesAvailable();
                dataResponse = serialPort->read(inByte);
                //                 dataResponse = devicePort->readAll();
                qDebug() << QString("\n <-- Response <----\n");
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

bool WatchDogs::openPort() {
    if (devicesCreated) {
        // Если девайс для работы с портом обявлен
        is_open = false;

        // Даем девайсу название порта
        serialPort->setPortName(com_Name);

        if (serialPort->open(QIODevice::ReadWrite)) {
            // Если Девайсу удалось открыть порт

            // Устанавливаем параметры открытия порта
            is_open = false;

            if (!serialPort->setDataBits(QSerialPort::Data8)) {
                return false;
            }
            if (!serialPort->setParity(QSerialPort::NoParity)) {
                return false;
            }
            if (!serialPort->setStopBits(QSerialPort::OneStop)) {
                return false;
            }
            if (!serialPort->setFlowControl(QSerialPort::NoFlowControl)) {
                return false;
            }
            if (!serialPort->setBaudRate(QSerialPort::Baud9600)) {
                return false;
            }

            qDebug() << "\nWatchDogs " << COSMP1::DeviceName << " to Port "
                     << serialPort->portName() << " open in " << serialPort->openMode();

            is_open = true;
        } else {
            is_open = false;
            qDebug() << "Error opened serial device " << serialPort->portName();
        }
    } else {
        is_open = false;
    }

    return is_open;
}

bool WatchDogs::isItYou(QStringList &comList, QString &wdName, QString &comStr, QString &wdComent) {
    wdName = WD_List.at(0);

    if ((wdName != "") && (comStr != "") && (comStr.contains("COM"))) {
        this->setPort(comStr);
        if (this->isItYou(wdComent)) {
            return true;
        }
    }

    int comLstC = comList.count();
    for (int comCount = 0; comCount < comLstC; comCount++) {

        QString vrmPort = comList.at(comCount);
        qDebug() << "--- com_count  - " << comCount;
        qDebug() << "--- vrm_Port    - " << vrmPort;

        this->setPort(vrmPort);

        if (this->isItYou(wdComent)) {
            comStr = vrmPort;
            return true;
        }
    }

    return false;
}

bool WatchDogs::isItYou(QString &wdComent) {
    bool resultP = false;
    // открываем модем для записи
    if (this->openPort()) {

        QByteArray cmdData;
        QByteArray answerData;

        WDProtocolCommands::Enum protocolCommand = WDProtocolCommands::GetID;

        this->processCommand(protocolCommand, cmdData, answerData);

        //        qDebug() <<
        //        "this->processCommand(protocolCommand,cmdData,answerData); - " <<
        //        answerData;

        if (QString(answerData).indexOf(COSMP1::DeviceID) == -1) {
            //            qDebug() << " WatchDogs::isItYou return false;";
        } else {
            //            qDebug() << " WatchDogs::isItYou return true;";

            wdComent = "WDT " + COSMP1::DeviceID;

            resultP = true;
        }
    }

    this->closePort();

    return resultP;
}

bool WatchDogs::processCommand(WDProtocolCommands::Enum aCommand,
                               const QByteArray &aCommandData,
                               QByteArray &aAnswerData) {
    Q_UNUSED(aCommandData)

    QByteArray commandData;

    for (int i = 0; i < COSMP1::PacketConstSize; ++i) {
        commandData.push_back(COSMP1::PacketConst[i]);
    }

    switch (aCommand) {
    case WDProtocolCommands::GetID: {
        commandData.push_back(QChar(COSMP1::Commands::GetID).cell());
    } break;

    case WDProtocolCommands::PCEnable: {
        commandData.push_back(QChar(COSMP1::Commands::PCEnable).cell());
    } break;

    case WDProtocolCommands::RebootPC: {
        commandData.push_back(QChar(COSMP1::Commands::RebootPC).cell());
    } break;

    case WDProtocolCommands::ResetModem: {
        commandData.push_back(QChar(COSMP1::Commands::ResetModem).cell());
    } break;

    case WDProtocolCommands::StartTimer: {
        commandData.push_back(QChar(COSMP1::Commands::StartTimer).cell());
    } break;

    case WDProtocolCommands::StopTimer: {
        commandData.push_back(QChar(COSMP1::Commands::StopTimer).cell());
    } break;
    }

    bool respData = false;

    this->sendCommand(commandData, true, 300, respData, aAnswerData, 0);

    return respData;
}

bool WatchDogs::toCommandExec(bool thread, WDProtocolCommands::Enum aCommand) {
    bool res = false;

    if (thread) {

        nowCommand = aCommand;
        this->start();
    } else {
        res = this->sendCommandToExec(aCommand);
    }

    return res;
}

void WatchDogs::run() {
    this->sendCommandToExec(nowCommand);
}

bool WatchDogs::sendCommandToExec(WDProtocolCommands::Enum aCommand) {
    bool respData = false;
    if (!this->isOpened()) {

        if (this->openPort()) {

            QByteArray cmdData;
            QByteArray answerData;

            // Отправляем команду
            respData = this->processCommand(aCommand, cmdData, answerData);

            emit commandDone(respData, aCommand);
        }

        this->closePort();
    }

    return respData;
}
