/* @file Базовый класс портов. */

#include <SDK/Drivers/Components.h>

#include <Hardware/IOPorts/IOPortBase.h>

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
IOPortBase::IOPortBase()
    : m_Type(EPortTypes::Unknown), m_DeviceIOLoging(ELoggingType::None), m_OpeningTimeout(0) {}

//--------------------------------------------------------------------------------
void IOPortBase::setOpeningTimeout(int aTimeout) {
    m_OpeningTimeout = aTimeout;
    setConfigParameter(CHardware::Port::OpeningTimeout, aTimeout);
}

//--------------------------------------------------------------------------------
QString IOPortBase::getDeviceType() {
    return CComponents::IOPort;
}

//--------------------------------------------------------------------------------
bool IOPortBase::clear() {
    return true;
}

//--------------------------------------------------------------------------------
QString IOPortBase::getName() const {
    return m_System_Name;
}

//--------------------------------------------------------------------------------
bool IOPortBase::release() {
    bool closingResult = close();
    bool result = MetaDevice::release();

    return closingResult && result;
}

//--------------------------------------------------------------------------------
EPortTypes::Enum IOPortBase::getType() {
    return m_Type;
}

//--------------------------------------------------------------------------------
void IOPortBase::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    MetaDevice::setDeviceConfiguration(aConfiguration);

    if (aConfiguration.contains(CHardwareSDK::System_Name)) {
        m_System_Name = aConfiguration[CHardwareSDK::System_Name].toString();
    }

    if (aConfiguration.contains(CHardware::Port::DeviceModelName)) {
        m_ConnectedDeviceName = aConfiguration[CHardware::Port::DeviceModelName].toString();
    }

    if (aConfiguration.contains(CHardware::Port::IOLogging)) {
        m_DeviceIOLoging = aConfiguration[CHardware::Port::IOLogging].value<ELoggingType::Enum>();
    }

    if (aConfiguration.contains(CHardware::Port::OpeningTimeout)) {
        m_OpeningTimeout = aConfiguration[CHardware::Port::OpeningTimeout].toInt();
    }
}

//--------------------------------------------------------------------------------
void IOPortBase::adjustData(const QStringList &aMine, const QStringList &aOther) {
    QVariantMap outDeviceData;
    outDeviceData.insert(CDeviceData::Ports::Mine, aMine.join("\n"));
    outDeviceData.insert(CDeviceData::Ports::Other, aOther.join("\n"));
    setConfigParameter(CHardwareSDK::DeviceData, outDeviceData);

    if (!isAutoDetecting()) {
        QString portData = outDeviceData[CDeviceData::Ports::Mine].toString();
        QString otherData = outDeviceData[CDeviceData::Ports::Other].toString();

        if (!portData.isEmpty()) {
            toLog(LogLevel::Normal, "Port data:\n" + portData);
        }

        if (!otherData.isEmpty()) {
            bool justConnected = getConfigParameter(CHardware::Port::JustConnected, false).toBool();
            LogLevel::Enum logLevel =
                (portData.isEmpty() && !justConnected) ? LogLevel::Normal : LogLevel::Debug;
            toLog(logLevel, "Port data additional:\n" + otherData);
        }
    }
}

//--------------------------------------------------------------------------------
