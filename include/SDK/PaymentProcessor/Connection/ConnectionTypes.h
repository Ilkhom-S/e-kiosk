#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

namespace EConnectionTypes
{
enum Enum
{
    Unknown = 0,
    Local,
    Dialup,
    GPRS,
    Ethernet,
    WiFi,
    Bluetooth
};

// Convert enum to string
inline QString toString(Enum aType)
{
    switch (aType)
    {
    case Local: return "Local";
    case Dialup: return "Dialup";
    case GPRS: return "GPRS";
    case Ethernet: return "Ethernet";
    case WiFi: return "WiFi";
    case Bluetooth: return "Bluetooth";
    default: return "Unknown";
    }
}

// Convert string to enum
inline Enum fromString(const QString &aString)
{
    if (aString == "Local") return Local;
    if (aString == "Dialup") return Dialup;
    if (aString == "GPRS") return GPRS;
    if (aString == "Ethernet") return Ethernet;
    if (aString == "WiFi") return WiFi;
    if (aString == "Bluetooth") return Bluetooth;
    return Unknown;
}
}