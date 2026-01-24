/* @file Асинхронный последовательный порт (платформо-независимый). */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVector>
#include <QtCore/QSet>
#include <QtCore/QUuid>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Drivers/IOPort/COMParameters.h>

// Project
#include <Hardware/IOPorts/IOPortBase.h>
#include <Hardware/IOPorts/IOPortGUIDs.h>

//--------------------------------------------------------------------------------
typedef QVector<QUuid> TUuids;

/// Буфер для чтения.
typedef QVector<char> TReadingBuffer;

/// Данные портов.
typedef QMap<QString, QString> TIOPortDeviceData;

//--------------------------------------------------------------------------------
/// Константы для работы с асинхронным COM-портом.
namespace CAsyncSerialPort {
    /// Коэффициент надежности.
    const double KSafety = 1.8;

    /// Коэффициент надежности таймаута фактического времени открытия порта.
    const double KOpeningTimeout = 1.5;

    /// Id для идентификации COM-портов.
    namespace Tags {
        /// Виртуальные COM-порты (через USB).
        inline QStringList Virtual() {
            return QStringList() << "USB"      /// Драйвер cp210x и совместимые.
                                 << "FTDI"     /// Чип FTDI.
                                 << "Virtual"; /// Что-то виртуальное.
        }

        /// Эмуляторы (программные) COM-порты.
        inline QStringList Emulator() {
            return QStringList() << "COM0COM"
                                 << "Emulator";
        }
    } // namespace Tags

    /// ACPI-устройства, такие как обычные COM-порты (не устройство расширения или виртуальный порт).
    const char GeneralRS232[] = "ACPI";

    /// Признаки невозможности ожидания результата GetOverlappedResult.
    const QStringList CannotWaitResult = QStringList() << "FTDI" << "LPC USB VCom Port" << "ATOL" << "MSTAR" << "CP210"
                                                       << "STMicroelectronics" << "Honeywell";

    /// Таймаут единичного чтения из порта, [мс].
    const int ReadingTimeout = 50;

    /// Таймаут открытия порта, [мс].
    const int OpeningTimeout = 1500;

    /// Таймаут открытия порта в процессе подключения, [мс].
    const int OnlineOpeningTimeout = 5 * 1000;

    /// Пауза для VCOM-портов между ожиданием и чтением данных, [мс].
    const int VCOMReadingPause = 3;
} // namespace CAsyncSerialPort

//--------------------------------------------------------------------------------
/// Платформо-независимый асинхронный последовательный порт.
/// Использует композицию для выбора правильной платформенной реализации.
class AsyncSerialPort : public IOPortBase {
    SET_SERIES("COM")

  public:
    AsyncSerialPort();
    virtual ~AsyncSerialPort();

    /// Возвращает список доступных в системе портов.
    static QStringList enumerateSystemNames();

    /// Опрашивает данные портов.
    virtual void initialize();

#pragma region SDK::Driver::IDevice
    /// Устанавливает конфигурацию устройству.
    virtual void setDeviceConfiguration(const QVariantMap &aConfiguration);

    /// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова initialize().
    virtual bool release();
#pragma endregion

#pragma region SDK::Driver::IIOPort
    /// Открыть порт.
    virtual bool open();

    /// Закрыть порт.
    virtual bool close();

    /// Очистить буферы порта.
    virtual bool clear();

    /// Установить параметры порта.
    virtual bool setParameters(const SDK::Driver::TPortParameters &aParameters);

    /// Получить параметры порта.
    virtual void getParameters(SDK::Driver::TPortParameters &aParameters);

    /// Прочитать данные.
    virtual bool read(QByteArray &aData, int aTimeout = DefaultReadTimeout, int aMinSize = 1);

    /// Передать данные.
    virtual bool write(const QByteArray &aData);

    /// Подключено новое устройство?
    virtual bool deviceConnected();

    /// Открыт?
    virtual bool opened();

    /// Порт существует?
    virtual bool isExist();
#pragma endregion

    /// Изменить таймаут выполнения зависоноопасной операции
    void changePerformingTimeout(const QString &aContext, int aTimeout, int aPerformingTime);

  public:
    /// Интерфейс платформенной реализации
    class ISerialPortImpl {
      public:
        virtual ~ISerialPortImpl() = default;

        virtual QStringList enumerateSystemNames() = 0;
        virtual void initialize() = 0;
        virtual void setDeviceConfiguration(const QVariantMap &aConfiguration) = 0;
        virtual bool release() = 0;
        virtual bool open() = 0;
        virtual bool close() = 0;
        virtual bool clear() = 0;
        virtual bool setParameters(const SDK::Driver::TPortParameters &aParameters) = 0;
        virtual void getParameters(SDK::Driver::TPortParameters &aParameters) = 0;
        virtual bool read(QByteArray &aData, int aTimeout, int aMinSize) = 0;
        virtual bool write(const QByteArray &aData) = 0;
        virtual bool deviceConnected() = 0;
        virtual bool opened() = 0;
        virtual bool isExist() = 0;
        virtual void changePerformingTimeout(const QString &aContext, int aTimeout, int aPerformingTime) = 0;
    };

  protected:
    /// Платформенная реализация (композиция вместо наследования)
    ISerialPortImpl *m_impl;
};

//--------------------------------------------------------------------------------
