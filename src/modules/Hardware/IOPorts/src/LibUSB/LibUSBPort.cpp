/* @file LibUSB-порт. */

#include "LibUSBPort.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/QMetaType>
#include <QtCore/QRegularExpression>

#include <Hardware/IOPorts/LibUSBUtils.h>
#include <iterator>

using namespace ProtocolUtils;

//--------------------------------------------------------------------------------
QRecursiveMutex LibUSBPort::m_DevicesPropertyMutex;

//--------------------------------------------------------------------------------
LibUSBPort::LibUSBPort() : m_Handle(nullptr), m_Exist(false), m_Device(nullptr) {
    m_Type = SDK::Driver::EPortTypes::USB;
    m_InitializationError = true;
}

//--------------------------------------------------------------------------------
void LibUSBPort::setDevice(libusb_device *aDevice) {
    m_Device = aDevice;
    m_DeviceProperties = getDevicesProperties(false)[aDevice];

    auto getEPLogData = [&](const CLibUSB::SEndPoint &aEP) -> QString {
        return QString("max packet size = %1, data = %2, transfer type = %3")
            .arg(aEP.maxPacketSize)
            .arg(toHexLog(aEP.data))
            .arg(CLibUSBUtils::TransferTypeDescriptions[uint8_t(aEP.transferType)]);
    };

    if (!m_DeviceProperties.valid()) {
        toLog(LogLevel::Error,
              QString("Port properties are wrong: VID = %1, PID = %2,\ndeviceToHost = "
                      "%3,\nhostToDevice = %4")
                  .arg(toHexLog(m_DeviceProperties.VID))
                  .arg(toHexLog(m_DeviceProperties.PID))
                  .arg(getEPLogData(m_DeviceProperties.deviceToHost))
                  .arg(getEPLogData(m_DeviceProperties.hostToDevice)));
    }
}

//--------------------------------------------------------------------------------
libusb_device *LibUSBPort::getDevice() const {
    return m_Device;
}

//--------------------------------------------------------------------------------
void LibUSBPort::initialize() {
    const libusb_version *versionData = libusb_get_version();

    if (versionData) {
        QString RCVersion = versionData->rc ? QString(versionData->rc).simplified() : "";
        QString description =
            versionData->describe ? QString(versionData->describe).simplified() : "";
        QString data = QString("version %1.%2.%3.%4%5%6")
                           .arg(versionData->major)
                           .arg(versionData->minor)
                           .arg(versionData->micro)
                           .arg(versionData->nano)
                           .arg(RCVersion.isEmpty() ? "" : (" " + RCVersion))
                           .arg(description.isEmpty() ? "" : QString(" (%1)").arg(description));

        setDeviceParameter(CHardwareSDK::LibraryVersion, data);
    }

    m_InitializationError = !LibUSBUtils::getContext(m_Log);

    if (m_InitializationError) {
        return;
    }

    CLibUSB::TDeviceProperties libUSBProperties = getDevicesProperties(false);

    QStringList mineData;
    QStringList otherData;

    for (auto it = libUSBProperties.begin(); it != libUSBProperties.end(); ++it) {
        QStringList &data = (it.key() == m_Device) ? mineData : otherData;
        data << LibUSBUtils::getPropertyLog(it->deviceData);
    }

    adjustData(mineData, otherData);
}

//--------------------------------------------------------------------------------
bool LibUSBPort::release() {
    bool closingResult = close();

    LibUSBUtils::releaseDeviceList();
    LibUSBUtils::releaseContext(m_Log);

    bool result = MetaDevice::release();

    return closingResult && result;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::opened() {
    return m_Handle;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::open() {
    if (m_Handle) {
        return true;
    }

    if (m_InitializationError || !m_Device) {
        return false;
    }

    setConfigParameter(CHardware::Port::JustConnected, false);

    if (!LIB_USB_CALL_LOG(m_Log, libusb_open, m_Device, &m_Handle)) {
        return false;
    }

    CLibUSB::SDeviceProperties deviceProperties = getDevicesProperties(false)[m_Device];
    QVariantMap &deviceData = deviceProperties.deviceData;

    auto getData = [&deviceData](const QString &aKey) -> QString {
        return deviceData.value(aKey).toString().simplified();
    };
    auto getFullData = [&deviceData, &getData](const QString &aKey1,
                                               const QString &aKey2) -> QString {
        QString result = QString("%1 = %2").arg(aKey1.toUpper()).arg(getData(aKey1));
        QString option = getData(aKey2);
        return option.isEmpty() ? result : QString("%1 (%2)").arg(result).arg(option);
    };

    toLog(LogLevel::Normal,
          QString("%1 with device %2 and %3 is opened")
              .arg(deviceProperties.portData)
              .arg(getFullData(CHardwareUSB::VID, DeviceUSBData::Vendor))
              .arg(getFullData(CHardwareUSB::PID, DeviceUSBData::Product)));

    int existingConfiguration = -1;

    if (!LIB_USB_CALL(libusb_get_configuration, m_Handle, &existingConfiguration)) {
        return false;
    }

    if ((existingConfiguration != 1) && !LIB_USB_CALL(libusb_set_configuration, m_Handle, 1)) {
        return false;
    }

    LIB_USB_CALL(libusb_set_auto_detach_kernel_driver, m_Handle, 1);

    if (!LIB_USB_CALL(libusb_claim_interface, m_Handle, 0)) {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::close() {
    bool result = true;
    bool beenOpened = m_Handle;

    if (!m_InitializationError && m_Handle) {
        result = LIB_USB_CALL(libusb_release_interface, m_Handle, 0);
        libusb_close(m_Handle);
    }

    if (result && beenOpened) {
        toLog(LogLevel::Normal,
              QString("Port %1 is closed.").arg(getDevicesProperties(false)[m_Device].portData));
    }

    m_Handle = nullptr;

    return result;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::checkExistence() {
    if (isExist()) {
        return true;
    }

    m_Exist = m_Devices.contains(m_Device);

    if (!m_Exist) {
        if (m_Device) {
            toLog(LogLevel::Error, "Port does not exist.");
        }

        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::checkReady() {
    if (!checkExistence()) {
        return false;
    }

    if (!m_Handle && !open()) {
        toLog(LogLevel::Error, "Port does not opened.");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::read(QByteArray &aData, int aTimeout, int aMinSize) {
    aData.clear();

    if (!checkReady()) {
        return false;
    }

    QElapsedTimer waitingTimer;
    waitingTimer.start();

    while ((waitingTimer.elapsed() < aTimeout) && (aData.size() < aMinSize)) {
        int received = 0;
        CLibUSB::SEndPoint &EP = m_DeviceProperties.deviceToHost;
        m_ReadingBuffer.fill(ASCII::NUL, EP.maxPacketSize);

        TResult result = LIB_USB_CALL(m_DeviceProperties.hostToDevice.processIO,
                                      m_Handle,
                                      EP.data,
                                      (unsigned char *)&m_ReadingBuffer[0],
                                      EP.maxPacketSize,
                                      &received,
                                      aTimeout);

        if (LIB_USB_SUCCESS(result)) {
            aData.append(m_ReadingBuffer.data(), received);
        } else if (result != LIBUSB_ERROR_TIMEOUT) {
            return false;
        }
    }

    if (m_DeviceIOLoging == ELoggingType::ReadWrite) {
        toLog(LogLevel::Normal,
              QString("%1: << {%2}").arg(m_ConnectedDeviceName).arg(aData.toHex().constData()));
    }

    return true;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::write(const QByteArray &aData) {
    if (aData.isEmpty()) {
        toLog(LogLevel::Normal, m_ConnectedDeviceName + ": written data is empty.");
        return false;
    }

    if (!checkReady()) {
        return false;
    }

    if (m_DeviceIOLoging != ELoggingType::None) {
        toLog(LogLevel::Normal,
              QString("%1: >> {%2}").arg(m_ConnectedDeviceName).arg(aData.toHex().constData()));
    }

    int partSize = m_DeviceProperties.hostToDevice.maxPacketSize;
    int parts = qCeil(double(aData.size()) / partSize);

    for (int i = 0; i < parts; ++i) {
        if (!perform_Write(aData.mid(i * partSize, partSize))) {
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::perform_Write(const QByteArray &aData) {
    int bytesWritten = 0;
    int actualSize = aData.size();
    int timeout = CLibUSBPort::writingTimeout(actualSize);

    TResult result = LIB_USB_CALL(m_DeviceProperties.deviceToHost.processIO,
                                  m_Handle,
                                  m_DeviceProperties.hostToDevice(),
                                  (unsigned char *)aData.data(),
                                  actualSize,
                                  &bytesWritten,
                                  timeout);

    if (!result) {
        return false;
    }

    if (bytesWritten != actualSize) {
        toLog(LogLevel::Normal,
              m_ConnectedDeviceName + QString(": %1 bytes instead of %2 bytes have been written.")
                                          .arg(bytesWritten)
                                          .arg(actualSize));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
TResult LibUSBPort::handleResult(const QString &aFunctionName, int aResult) {
    TResult result = LibUSBUtils::logAnswer(aFunctionName, aResult, m_Log);

    if (CLibUSBPort::DisappearingErrors.contains(aResult) &&
        !getDevicesProperties(true).contains(m_Device)) {
        close();

        m_Exist = false;
    }

    return result;
}

//--------------------------------------------------------------------------------
bool LibUSBPort::deviceConnected() {
    CLibUSB::TDeviceProperties devicesProperties = getDevicesProperties(true);
    int result =
        (devicesProperties.size() - m_DevicesProperties.size()) * m_DevicesProperties.size();

    m_DevicesProperties = devicesProperties;

    if (result > 0) {
        setConfigParameter(CHardware::Port::JustConnected, true);

        return true;
    }

    checkExistence();

    return false;
};

//--------------------------------------------------------------------------------
bool LibUSBPort::isExist() {
    return m_Exist;
}

//--------------------------------------------------------------------------------
CLibUSB::TDeviceProperties LibUSBPort::getDevicesProperties(bool aForce) {
    QMutexLocker locker(&m_DevicesPropertyMutex);

    static CLibUSB::TDeviceProperties properties;

    if ((!properties.isEmpty() && !aForce) ||
        !LibUSBUtils::getDevicesProperties(properties, aForce)) {
        return properties;
    }

    for (auto it = properties.begin(); it != properties.end();) {
        QString deviceProduct = it->deviceData.value(DeviceUSBData::Product).toString().toLower();
        bool needErase = !it->VID || !it->PID || deviceProduct.contains("mouse");
        it = needErase ? properties.erase(it) : std::next(it);
    }

    m_Devices = properties.keys();

    return properties;
}

//--------------------------------------------------------------------------------
