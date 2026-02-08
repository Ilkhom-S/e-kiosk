/* @file LibUSB-порт. */

#pragma once

// LibUSB SDK
#pragma warning(push, 1)
#include "libusb.h"
#pragma warning(pop)

#include <QtCore/QRecursiveMutex>
#include <QtCore/QVector>
#include <QtCore/qmath.h>

#include "Hardware/Common/CommandResultData.h"
#include "Hardware/IOPorts/IOPortBase.h"
#include "Hardware/IOPorts/LibUSBDeviceDataTypes.h"

#define LIB_USB_CALL(aName, ...) handleResult(#aName, aName(__VA_ARGS__))

//--------------------------------------------------------------------------------
namespace CLibUSBPort {
/// Таймаут отправки 1 байта, [мс].
const double ByteTimeout = 0.01;

/// Таймаут системных операций при отправке данных, [мс].
const double SystemTimeout = 1;

/// Таймаут для отправки данных, [мс].
inline int writingTimeout(int aSize) {
    return qCeil(SystemTimeout + aSize * ByteTimeout);
}

/// Ошибки пропажи порта.
const QVector<int> DisappearingErrors =
    QVector<int>()
    << LIBUSB_ERROR_IO         /// Ошибка ввода/вывода
    << LIBUSB_ERROR_NO_DEVICE; /// Устройство отсутствует (возможно, оно было отсоединено)
} // namespace CLibUSBPort

//--------------------------------------------------------------------------------
class LibUSBPort : public IOPortBase {
    SET_SERIES("LibUSB")

public:
    LibUSBPort();

    /// Опрашивает данные портов.
    virtual void initialize();

    /// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова
    /// initialize().
    virtual bool release();

    /// Открыть порт.
    virtual bool open();

    /// Закрыть порт.
    virtual bool close();

    /// Прочитать данные.
    virtual bool read(QByteArray &aData, int aTimeout = DefaultReadTimeout, int aMinSize = 1);

    /// Передать данные.
    virtual bool write(const QByteArray &aData);

    /// Установить устройство.
    void setDevice(libusb_device *aDevice);

    /// Получить устройство.
    libusb_device *getDevice() const;

    /// Подключено новое устройство?
    virtual bool deviceConnected();

    /// Открыт?
    virtual bool opened();

    /// Порт существует?
    virtual bool isExist();

    /// Получить системные свойства устройств.
    CLibUSB::TDeviceProperties getDevicesProperties(bool aForce);

protected:
    /// Идентификация.
    virtual bool checkExistence();

    /// Проверить готовность порта.
    virtual bool checkReady();

    /// Передать данные.
    bool performWrite(const QByteArray &aData);

    /// Обработать результат выполнения функции LibUSB.
    TResult handleResult(const QString &aFunctionName, int aResult);

    /// Список данных соединений.
    QList<libusb_device *> m_Devices;

    /// Устройство.
    libusb_device *m_Device;

    /// Буфер для чтения.
    typedef QVector<char> TReadingBuffer;
    TReadingBuffer m_ReadingBuffer;

    /// Handle устройства.
    libusb_device_handle *m_Handle;

    /// Данные устройств.
    CLibUSB::TDeviceProperties m_DevicesProperties;

    /// Параметры устройства.
    CLibUSB::SDeviceProperties m_DeviceProperties;

    /// Существует в системе.
    bool m_Exist;

    /// Мьютекс для защиты статических пропертей портов.
    static QRecursiveMutex m_DevicesPropertyMutex;
};

//--------------------------------------------------------------------------------
