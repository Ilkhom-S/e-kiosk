/* @file Описание типов устройств. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QString>

#include <SDK/Drivers/Components.h>

namespace SDK {
namespace Driver {

/// Возможные типы устройств, должны быть согласованы с путями драйверов.
namespace EDeviceType {
enum Enum {
    BillAcceptor,
    CoinAcceptor,
    Printer,
    Watchdog,
    Modem,
    Scanner,
    Virtual,
    CardReader,
    FiscalRegistrator,
    DocumentPrinter,
    Dispenser,
    Health
};

namespace {
/// Преобразует строку в тип устройства.
Enum from_String(const QString &aDeviceType) {
    static QMap<QString, Enum> translation;

    if (translation.isEmpty()) {
        translation[CComponents::BillAcceptor] = BillAcceptor;
        translation[CComponents::CoinAcceptor] = CoinAcceptor;
        translation[CComponents::Printer] = Printer;
        translation[CComponents::FiscalRegistrator] = FiscalRegistrator;
        translation[CComponents::DocumentPrinter] = DocumentPrinter;
        translation[CComponents::Watchdog] = Watchdog;
        translation[CComponents::Modem] = Modem;
        translation[CComponents::Scanner] = Scanner;
        translation[CComponents::CardReader] = CardReader;
        translation[CComponents::Dispenser] = Dispenser;
        translation[CComponents::Health] = Health;
    }

    return translation.contains(aDeviceType) ? translation.value(aDeviceType) : Virtual;
}
} // namespace
} // namespace EDeviceType

//---------------------------------------------------------------------------
} // namespace Driver
} // namespace SDK

//---------------------------------------------------------------------------
