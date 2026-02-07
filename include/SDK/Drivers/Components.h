/* @file Список компонентов для разработки плагинов. */

#pragma once

#include <QtCore/QString>

namespace SDK {
namespace Driver {

//---------------------------------------------------------------------------
/// Драйверы являются общим расширением.
extern const char Application[];

//---------------------------------------------------------------------------
/// Список возможных компонентов.
namespace CComponents {
extern const char Driver[];
extern const char IOPort[];
extern const char BillAcceptor[];
extern const char CoinAcceptor[];
extern const char Dispenser[];
extern const char Printer[];
extern const char FiscalRegistrator[];
extern const char DocumentPrinter[];
extern const char Watchdog[];
extern const char Modem[];
extern const char Scanner[];
extern const char CardReader[];
extern const char Health[];
extern const char Camera[];
extern const char Token[];

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
