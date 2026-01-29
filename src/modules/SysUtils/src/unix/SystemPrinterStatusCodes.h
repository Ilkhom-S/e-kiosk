/* @file Спецификация статусов системного принтера для Linux/macOS (CUPS/IPP).
 */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

namespace CUnixPrinter
{
    // Пример: коды CUPS/IPP и их соответствие внутренним статусам
    enum StatusCode
    {
        Enabled = 0,
        Disabled = 1,
        Idle = 2,
        Printing = 3,
        Unknown = 100
    };

    static const QMap<QString, StatusCode> StatusMap = {
        {"enabled", Enabled}, {"disabled", Disabled}, {"idle", Idle}, {"printing", Printing}};

    static QString statusToString(StatusCode code)
    {
        switch (code)
        {
            case Enabled:
                return "enabled";
            case Disabled:
                return "disabled";
            case Idle:
                return "idle";
            case Printing:
                return "printing";
            default:
                return "unknown";
        }
    }
} // namespace CUnixPrinter
