/* @file Кроссплатформенная реализация USB-порта. */

#pragma once
#include <QtCore/QMutex>
#include <QtSerialPort/QSerialPortInfo>

#include "Hardware/IOPorts/AsyncSerialPort.h"
#include "Hardware/IOPorts/DeviceProperties.h" // Для TWinDeviceProperties

//--------------------------------------------------------------------------------
namespace CUSBPort {
const int OpeningPause = 500;

namespace DeviceTags {
extern const char ACPI[];
extern const char Mouse[];
extern const char USBPDO[];
} // namespace DeviceTags

const int DefaultMaxReadSize = 1024;
} // namespace CUSBPort

//--------------------------------------------------------------------------------
class USBPort : public AsyncSerialPort {
    SET_SERIES("USB")

public:
    USBPort();
    virtual ~USBPort() = default;

    /// Очистить буферы порта.
    virtual bool clear() override;

    /// Получить системные свойства устройств (кроссплатформенно через QSerialPortInfo).
    /// Мы сохраняем тип TWinDeviceProperties для совместимости, но наполняем его данными Qt.
    TDeviceProperties getDevicesProperties(bool aForce, bool aPDODetecting = false);

protected:
    /// Проверить готовность порта.
    virtual bool checkReady();

    /// Открыть порт.
    virtual bool perform_Open();

    /// Прочитать данные.
    virtual bool processReading(QByteArray &aData, int aTimeout);

    /// Мьютекс для защиты общих данных.
    static QMutex m_System_PropertyMutex;
};
