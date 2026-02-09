/* @file Список компонентов для разработки плагинов. */

#pragma once

#include <QtCore/QString>

namespace SDK {
namespace Driver {

//---------------------------------------------------------------------------
/// Драйверы являются общим расширением.
constexpr const char Application[] = "Common";

//---------------------------------------------------------------------------
/// Список возможных компонентов.
namespace CComponents {
constexpr const char Driver[] = "Driver";
constexpr const char IOPort[] = "IOPort";
constexpr const char BillAcceptor[] = "BillAcceptor";
constexpr const char CoinAcceptor[] = "CoinAcceptor";
constexpr const char Dispenser[] = "Dispenser";
constexpr const char Printer[] = "Printer";
constexpr const char FiscalRegistrator[] = "FiscalRegistrator";
constexpr const char DocumentPrinter[] = "DocumentPrinter";
constexpr const char Watchdog[] = "Watchdog";
constexpr const char Modem[] = "Modem";
constexpr const char Scanner[] = "Scanner";
constexpr const char CardReader[] = "CardReader";
constexpr const char Health[] = "Health";
constexpr const char Camera[] = "Camera";
constexpr const char Token[] = "Token";

/// Проверяет, является ли тип устройства принтером.
inline bool isPrinter(const QString &aDeviceType) {
    return aDeviceType == Printer || aDeviceType == DocumentPrinter ||
           aDeviceType == FiscalRegistrator;
}
} // namespace CComponents

//---------------------------------------------------------------------------
} // namespace Driver
} // namespace SDK

//---------------------------------------------------------------------------
