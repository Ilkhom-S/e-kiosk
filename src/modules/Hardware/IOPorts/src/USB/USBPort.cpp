/* @file Кроссплатформенная реализация USB-порта на базе QtSerialPort. */

#include "USBPort.h"

#include <QtCore/QMutexLocker>
#include <QtSerialPort/QSerialPortInfo>

using namespace SDK::Driver;

// Инициализация рекурсивного мьютекса (совместимо с Qt 5.15 и 6)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
QMutex USBPort::mSystemPropertyMutex;
#else
QMutex USBPort::mSystemPropertyMutex(QMutex::Recursive);
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
    mType = EPortTypes::USB;
    // Удалены привязки к Windows GUID и DWORD свойствам
    setOpeningTimeout(CAsyncSerialPort::OpeningTimeout + CUSBPort::OpeningPause);
}

//--------------------------------------------------------------------------------
bool USBPort::performOpen() {
    if (!checkReady()) {
        return false;
    }

    // Небольшая пауза перед открытием для стабилизации USB-стека
    SleepHelper::msleep(CUSBPort::OpeningPause);

    // В Qt реализация открытия порта скрыта внутри QSerialPort
    // mSerialPort — это экземпляр QSerialPort, который должен быть в AsyncSerialPort
    this->mSerialPort.setPortName(mSystemName);

    // Настраиваем стандартные параметры (могут быть переопределены позже)
    this->mSerialPort.setBaudRate(QSerialPort::Baud9600);
    this->mSerialPort.setDataBits(QSerialPort::Data8);
    this->mSerialPort.setParity(QSerialPort::NoParity);
    this->mSerialPort.setStopBits(QSerialPort::OneStop);
    this->mSerialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (this->mSerialPort.open(QIODevice::ReadWrite)) {
        return true;
    }

    handleError(this->mSerialPort.errorString());
    return false;
}

//--------------------------------------------------------------------------------
bool USBPort::checkReady() {
    // Обновляем список доступных имен в системе кроссплатформенно
    TWinDeviceProperties props = getDevicesProperties(true);
    mExist = props.contains(mSystemName);

    if (!mExist) {
        setOpeningTimeout(CAsyncSerialPort::OnlineOpeningTimeout);
        toLog(LogLevel::Error, QStringLiteral("Port %1 does not exist.").arg(mSystemName));
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool USBPort::clear() {
    if (this->mSerialPort.isOpen()) {
        return this->mSerialPort.clear();
    }
    return true;
}

//--------------------------------------------------------------------------------
bool USBPort::processReading(QByteArray &aData, int aTimeout) {
    aData.clear();
    QMutexLocker locker(&mReadMutex);

    if (!this->mSerialPort.isOpen()) {
        return false;
    }

    // Ожидание поступления данных (кроссплатформенная замена Overlapped ожидания)
    if (this->mSerialPort.waitForReadyRead(aTimeout)) {
        aData = this->mSerialPort.readAll();
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------
TDeviceProperties USBPort::getDevicesProperties(bool aForce, bool aPDODetecting) {
    QMutexLocker locker(&mSystemPropertyMutex);
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

    mSystemNames = properties.keys();
    return properties;
}
