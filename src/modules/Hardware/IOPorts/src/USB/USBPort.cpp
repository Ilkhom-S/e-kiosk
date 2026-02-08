/* @file Кроссплатформенная реализация USB-порта на базе QtSerialPort. */

#include "USBPort.h"

#include <QtCore/QMutexLocker>
#include <QtSerialPort/QSerialPortInfo>

using namespace SDK::Driver;

// Инициализация рекурсивного мьютекса (совместимо с Qt 5.15 и 6)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QMutex USBPort::m_System_PropertyMutex;
#else
QMutex USBPort::m_System_PropertyMutex(QMutex::Recursive);
#endif

//--------------------------------------------------------------------------------
namespace CUSBPort {
namespace DeviceTags {
const char ACPI[] = "ACPI";
const char Mouse[] = "Mouse";
const char USBPDO[] = "USBPDO";
} // namespace DeviceTags
} // namespace CUSBPort

//--------------------------------------------------------------------------------
    m_Type = EPortTypes::USB;
    // Удалены привязки к Windows GUID и DWORD свойствам
    setOpeningTimeout(CAsyncSerialPort::OpeningTimeout + CUSBPort::OpeningPause);
}

//--------------------------------------------------------------------------------
bool USBPort::perform_Open() {
    if (!checkReady()) {
        return false;
    }

    // Небольшая пауза перед открытием для стабилизации USB-стека
    SleepHelper::msleep(CUSBPort::OpeningPause);

    // В Qt реализация открытия порта скрыта внутри QSerialPort
    // m_SerialPort — это экземпляр QSerialPort, который должен быть в AsyncSerialPort
    this->m_SerialPort.setPortName(m_System_Name);

    // Настраиваем стандартные параметры (могут быть переопределены позже)
    this->m_SerialPort.setBaudRate(QSerialPort::Baud9600);
    this->m_SerialPort.setDataBits(QSerialPort::Data8);
    this->m_SerialPort.setParity(QSerialPort::NoParity);
    this->m_SerialPort.setStopBits(QSerialPort::OneStop);
    this->m_SerialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (this->m_SerialPort.open(QIODevice::ReadWrite)) {
        return true;
    }

    handleError(this->m_SerialPort.errorString());
    return false;
}

//--------------------------------------------------------------------------------
bool USBPort::checkReady() {
    // Обновляем список доступных имен в системе кроссплатформенно
    TWinDeviceProperties props = getDevicesProperties(true);
    m_Exist = props.contains(m_System_Name);

    if (!m_Exist) {
        setOpeningTimeout(CAsyncSerialPort::OnlineOpeningTimeout);
        toLog(LogLevel::Error, QStringLiteral("Port %1 does not exist.").arg(m_System_Name));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool USBPort::clear() {
    if (this->m_SerialPort.isOpen()) {
        return this->m_SerialPort.clear();
    }
    return true;
}

//--------------------------------------------------------------------------------
bool USBPort::processReading(QByteArray &aData, int aTimeout) {
    aData.clear();
    QMutexLocker locker(&m_ReadMutex);

    if (!this->m_SerialPort.isOpen()) {
        return false;
    }

    // Ожидание поступления данных (кроссплатформенная замена Overlapped ожидания)
    if (this->m_SerialPort.waitForReadyRead(aTimeout)) {
        aData = this->m_SerialPort.readAll();
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------
TDeviceProperties USBPort::getDevicesProperties(bool aForce, bool aPDODetecting) {
    QMutexLocker locker(&m_System_PropertyMutex);
    static TDeviceProperties properties;

    if (!properties.isEmpty() && !aForce) {
        return properties;
    }

    properties.clear();

    // Используем наш новый кроссплатформенный описатель свойств
    DeviceProperties dp;

    const auto availablePorts = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : availablePorts) {
        SDeviceProperties props;
        props.path = info.portName();
        props.VID = info.hasVendorIdentifier() ? info.vendorIdentifier() : 0;
        props.PID = info.hasProductIdentifier() ? info.productIdentifier() : 0;

        // Фильтрация через описание и производителя (кроссплатформенно)
        QString description = info.description().toUpper();
        QString manufacturer = info.manufacturer().toUpper();

        if (description.contains(QStringLiteral("MOUSE")) ||
            manufacturer.contains(QStringLiteral("ACPI"))) {
            continue;
        }

        // Наполняем мапу свойств, используя кроссплатформенные ключи DeviceProperties.
        // Это позволяет методам логирования и фильтрации в других частях SDK
        // работать корректно, не зная о специфике Windows/Linux.
        props.data[dp[CDeviceProperties::FriendlyName]] = info.description();
        props.data[dp[CDeviceProperties::DeviceDesc]] = info.description();
        props.data[dp[CDeviceProperties::Manufacturer]] = info.manufacturer();
        props.data[dp[CDeviceProperties::Enumerator]] = QStringLiteral("USB");

        // Для совместимости с логикой PDODetecting на Windows 7
        props.data[dp[CDeviceProperties::PhysName]] = info.portName();

        properties.insert(info.portName(), props);
    }

    m_System_Names = properties.keys();
    return properties;
}
