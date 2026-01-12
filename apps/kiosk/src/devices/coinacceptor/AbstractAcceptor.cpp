// Project
#include "AbstractAcceptor.h"

BaseAcceptorDevices::BaseAcceptorDevices(QObject *parent) : QThread(parent)
{
	Debuger = 0;
	devicesCreated = false;
	this->createDevicePort();
}

bool BaseAcceptorDevices::createDevicePort()
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

bool BaseAcceptorDevices::isOpened()
{
	if (serialPort->isOpen())
		is_open = true;
	else
		is_open = false;

	return is_open;
}

void BaseAcceptorDevices::setPortName(const QString com_Name)
{
	comName = com_Name;
}

void BaseAcceptorDevices::setPartNumber(const QString partNumber)
{

	part_number = partNumber;
}

bool BaseAcceptorDevices::closePort()
{
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

bool BaseAcceptorDevices::sendCommand(QByteArray dataRequest, bool getResponse, int timeResponse,
									  QByteArray &dataResponse, int timeSleep)
{
	bool respOk = false;

	if (isOpened())
	{
		// Если девайс открыт
		respOk = false;

		serialPort->write(dataRequest);

		msleep(timeSleep);

		if (getResponse)
		{
			QByteArray data;

			while (serialPort->waitForReadyRead(timeResponse))
			{
				data += serialPort->readAll();
			}

			dataResponse = data;

			respOk = true;
		}

		return respOk;
	}

	return respOk;
}
