#pragma once

#include <QtCore/QSharedPointer>

#include <Hardware/IOPorts/COMPortBase.h>

class SerialDevice;

//-----------------------------------------------------------------------------
namespace CCom_PortLin {
const QString LogName = "Com_PortLin";
}; // namespace CCom_PortLin

//-----------------------------------------------------------------------------
class Com_PortLin : public Com_PortBase {
public:
    Com_PortLin(const QString &aFilePath);
    virtual ~Com_PortLin();

    virtual bool open();
    virtual bool init();
    virtual bool release();
    virtual bool clear();
    virtual bool
    readData(QByteArray &aData, unsigned int aMaxSize, bool aIsTimeOut, bool aIsFirstData);
    virtual int writeData(const QByteArray &aData);
    virtual bool setBaudRate(PortParameters::BaudRate::Enum aBaudRate);
    virtual bool setStopBits(PortParameters::StopBits::Enum aStopBits);
    virtual bool setParity(PortParameters::Parity::Enum aParity);
    virtual bool setDTR(PortParameters::DTR::Enum aDTR);
    virtual bool setRTS(PortParameters::RTS::Enum aRTS);
    virtual void setTimeOut(int aMsecs);

private:
    QSharedPointer<SerialDevice> m_device;
    QString m_deviceName;
};

//-----------------------------------------------------------------------------
