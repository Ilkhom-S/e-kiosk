#include "AbstractAcceptor.h"

BaseAcceptorDevices::BaseAcceptorDevices(QObject *parent) : QThread(parent), Debugger(0), devicesCreated(false) {
    
    
    this->createDevicePort();
}

bool BaseAcceptorDevices::createDevicePort() {
    serialPort = new QSerialPort(this);

    devicesCreated = serialPort != nullptr;

    return devicesCreated;
}

bool BaseAcceptorDevices::isOpened() {
    is_open = serialPort->isOpen();

    return is_open;
}

void BaseAcceptorDevices::setPortName(const QString comName) {
    this->com_Name = comName;
}

void BaseAcceptorDevices::setPartNumber(const QString partNumber) {

    part_number = partNumber;
}

bool BaseAcceptorDevices::closePort() {
    if (!isOpened()) {
        return true;
    }

    //    serialPort->reset();
    serialPort->flush();
    serialPort->close();
    is_open = false;
    return true;
}

bool BaseAcceptorDevices::sendCommand(QByteArray dataRequest,
                                      bool getResponse,
                                      int timeResponse,
                                      QByteArray &dataResponse,
                                      int timeSleep) {
    bool respOk = false;

    if (isOpened()) {
        // Если девайс открыт
        respOk = false;

        serialPort->write(dataRequest);

        msleep(timeSleep);

        if (getResponse) {
            QByteArray data;

            while (serialPort->waitForReadyRead(timeResponse)) {
                data += serialPort->readAll();
            }

            dataResponse = data;

            respOk = true;
        }

        return respOk;
    }

    return respOk;
}
