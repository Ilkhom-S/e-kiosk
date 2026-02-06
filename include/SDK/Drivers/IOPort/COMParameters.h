/* @file Параметры COM-порта. */

#pragma once

#include <QtCore/QString>

namespace SDK {
namespace Driver {
namespace IOPort {
namespace COM {

//--------------------------------------------------------------------------------
/// Настройки чётности.
namespace EParity {
enum Enum { No, Odd, Even, Mark, Space };

/// Преобразовать в строку.
inline static const char *EnumToString(int aParity) {
    switch (aParity) {
    case No:
        return "no";
    case Odd:
        return "odd";
    case Even:
        return "even";
    case Mark:
        return "mark";
    case Space:
        return "space";
    }

    return "invalid";
}
} // namespace EParity

//--------------------------------------------------------------------------------
/// Управление линией DTR.
namespace EDTRControl {
enum Enum { Disable, Enable, Handshake };

inline static const char *EnumToString(int aDTR) {
    switch (aDTR) {
    case Disable:
        return "Disable";
    case Enable:
        return "Enable";
    case Handshake:
        return "Handshake";
    }

    return "invalid";
}
} // namespace EDTRControl

//--------------------------------------------------------------------------------
/// Управление линией RTS.
namespace ERTSControl {
enum Enum { Disable, Enable, Handshake, Toggle };

/// Преобразовать в строку.
inline static const char *EnumToString(int aRTS) {
    switch (aRTS) {
    case Disable:
        return "Disable";
    case Enable:
        return "Enable";
    case Handshake:
        return "Handshake";
    case Toggle:
        return "Toggle";
    }

    return "invalid";
}
} // namespace ERTSControl

//--------------------------------------------------------------------------------
/// Скорость обмена данными.
namespace EBaudRate {
enum Enum {
    BR4800 = 4800,
    BR9600 = 9600,
    BR14400 = 14400,
    BR19200 = 19200,
    BR38400 = 38400,
    BR57600 = 57600,
    BR115200 = 115200
};

/// Преобразовать в строку.
inline static const char *EnumToString(int aBaudRate) {
    switch (aBaudRate) {
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
    case BR57600:
        return "57600";
    case BR115200:
        return "115200";
    }

    return "invalid";
}
} // namespace EBaudRate

//--------------------------------------------------------------------------------
/// Количество стоповых бит.
namespace EStopBits {
enum Enum { One, One5, Two };

/// Преобразовать в строку.
inline static const char *EnumToString(int aBaudRate) {
    switch (aBaudRate) {
    case One:
        return "1";
    case One5:
        return "1.5";
    case Two:
        return "2";
    }

    return "invalid";
}
} // namespace EStopBits

//--------------------------------------------------------------------------------
/// Параметры COM-порта.
namespace EParameters {
enum Enum {
    BaudRate, /// Скорость передачи данных.
    StopBits, /// Количество стоповых бит.
    Parity,   /// Схема контроля четности.
    DTR,      /// Управление модемной линией DTR.
    RTS,      /// Управление модемной линией RTS.
    ByteSize, /// Число информационных бит в передаваемых и принимаемых байтах.
    XOn,      /// Символ Xon, используемый для регулирования приема/передачи.
    XOff,     /// Символ Xoff, используемый для регулирования приема/передачи.
};

/// Преобразовать в строку.
inline static const char *EnumToString(int aParameter) {
    switch (aParameter) {
    case BaudRate:
        return "baud rate";
    case StopBits:
        return "stop bits";
    case Parity:
        return "parity";
    case DTR:
        return "DTR";
    case RTS:
        return "RTS";
    case ByteSize:
        return "byte size";
    }

    return "invalid";
}
} // namespace EParameters

/// Описание параметра.
inline static QString parameterDescription(int aParameter, int aValue) {
    switch (aParameter) {
    case EParameters::BaudRate:
        return EBaudRate::EnumToString(aValue);
    case EParameters::StopBits:
        return EStopBits::EnumToString(aValue);
    case EParameters::Parity:
        return EParity::EnumToString(aValue);
    case EParameters::DTR:
        return EDTRControl::EnumToString(aValue);
    case EParameters::RTS:
        return ERTSControl::EnumToString(aValue);
    case EParameters::ByteSize:
        return QString::number(aValue);
    }

    return "invalid value";
}

} // namespace COM
} // namespace IOPort
} // namespace Driver
} // namespace SDK

//--------------------------------------------------------------------------------
