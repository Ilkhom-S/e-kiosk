/* @file Базовый COM-порт. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <Common/QtHeadersEnd.h>

// Forward declarations
class IRegistry;

//--------------------------------------------------------------------------------
/// Параметры порта.
namespace PortParameters {
    //--------------------------------------------------------------------------------
    /// Скорость передачи данных.
    namespace BaudRate {
        enum Enum {
            BR110 = 110,
            BR300 = 300,
            BR600 = 600,
            BR1200 = 1200,
            BR2400 = 2400,
            BR4800 = 4800,
            BR9600 = 9600,
            BR14400 = 14400,
            BR19200 = 19200,
            BR38400 = 38400,
            BR56000 = 56000,
            BR57600 = 57600,
            BR115200 = 115200,
            BR128000 = 128000,
            BR256000 = 256000
        };

        /// Преобразовать в строку.
        inline static const char *toString(Enum aValue) {
            switch (aValue) {
                case BR110:
                    return "110";
                case BR300:
                    return "300";
                case BR600:
                    return "600";
                case BR1200:
                    return "1200";
                case BR2400:
                    return "2400";
                case BR4800:
                    return "4800";
                case BR9600:
                    return "9600";
                case BR14400:
                    return "14400";
                case BR19200:
                    return "19200";
                case BR38400:
                    return "38400";
                case BR56000:
                    return "56000";
                case BR57600:
                    return "57600";
                case BR115200:
                    return "115200";
                case BR128000:
                    return "128000";
                case BR256000:
                    return "256000";
                default:
                    return "unknown";
            }
        }
    } // namespace BaudRate

    //--------------------------------------------------------------------------------
    /// Стоповые биты.
    namespace StopBits {
        enum Enum { One, Two };

        /// Преобразовать в строку.
        inline static const char *toString(Enum aValue) {
            switch (aValue) {
                case One:
                    return "1";
                case Two:
                    return "2";
                default:
                    return "unknown";
            }
        }
    } // namespace StopBits

    //--------------------------------------------------------------------------------
    /// Чётность.
    namespace Parity {
        enum Enum { PNo, PEven, POdd, PSpace };

        /// Преобразовать в строку.
        inline static const char *toString(Enum aValue) {
            switch (aValue) {
                case PNo:
                    return "no";
                case PEven:
                    return "even";
                case POdd:
                    return "odd";
                case PSpace:
                    return "space";
                default:
                    return "unknown";
            }
        }
    } // namespace Parity

    //--------------------------------------------------------------------------------
    /// Управление DTR.
    namespace DTR {
        enum Enum { Disable, Enable, Handshake };

        /// Преобразовать в строку.
        inline static const char *toString(Enum aValue) {
            switch (aValue) {
                case Disable:
                    return "disable";
                case Enable:
                    return "enable";
                case Handshake:
                    return "handshake";
                default:
                    return "unknown";
            }
        }
    } // namespace DTR

    //--------------------------------------------------------------------------------
    /// Управление RTS.
    namespace RTS {
        enum Enum { Disable, Enable, Handshake };

        /// Преобразовать в строку.
        inline static const char *toString(Enum aValue) {
            switch (aValue) {
                case Disable:
                    return "disable";
                case Enable:
                    return "enable";
                case Handshake:
                    return "handshake";
                default:
                    return "unknown";
            }
        }
    } // namespace RTS
} // namespace PortParameters

//--------------------------------------------------------------------------------
/// Параметры COM-порта.
struct SComPortParameters {
    int portNumber = 0;
    PortParameters::BaudRate::Enum baudRate = PortParameters::BaudRate::BR9600;
    PortParameters::StopBits::Enum stopBits = PortParameters::StopBits::One;
    PortParameters::Parity::Enum parity = PortParameters::Parity::PNo;
    PortParameters::DTR::Enum dtrControl = PortParameters::DTR::Disable;
    PortParameters::RTS::Enum rtsControl = PortParameters::RTS::Disable;
};

//--------------------------------------------------------------------------------
/// Базовый класс для COM-портов.
class ComPortBase {
  public:
    ComPortBase() = default;
    virtual ~ComPortBase() = default;

    /// Открыть порт.
    virtual bool open() = 0;

    /// Инициализировать порт.
    virtual bool init() = 0;

    /// Освободить порт.
    virtual bool release() = 0;

    /// Очистить буферы порта.
    virtual bool clear() = 0;

    /// Прочитать данные.
    virtual bool readData(QByteArray &aData, unsigned int aMaxSize, bool aIsTimeOut, bool aIsFirstData) = 0;

    /// Записать данные.
    virtual int writeData(const QByteArray &aData) = 0;

    /// Установить скорость передачи.
    virtual bool setBaudRate(PortParameters::BaudRate::Enum aBaudRate) = 0;

    /// Установить стоповые биты.
    virtual bool setStopBits(PortParameters::StopBits::Enum aStopBits) = 0;

    /// Установить чётность.
    virtual bool setParity(PortParameters::Parity::Enum aParity) = 0;

    /// Установить управление DTR.
    virtual bool setDTR(PortParameters::DTR::Enum aDTR) = 0;

    /// Установить управление RTS.
    virtual bool setRTS(PortParameters::RTS::Enum aRTS) = 0;

    /// Установить таймаут.
    virtual void setTimeOut(int aMsecs) = 0;

  protected:
    /// Параметры COM-порта.
    SComPortParameters m_COMParameters;

    /// Параметры реестра.
    IRegistry *m_registryParameters = nullptr;

    /// Флаг открытия порта.
    bool m_isPortOpen = false;
};

//--------------------------------------------------------------------------------
