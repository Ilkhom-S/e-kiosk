/* @file Интерфейс адаптера настроек. */

#pragma once

// Stl
#include <iostream>
#include <string>

class QString;

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
namespace CAdapterNames {
const char TerminalAdapter[] = "TerminalSettings";
const char DealerAdapter[] = "DealerSettings";
const char UserAdapter[] = "UserSettings";
const char Directory[] = "Directory";
const char Extensions[] = "Extensions";
}; // namespace CAdapterNames

//---------------------------------------------------------------------------
class ISettingsAdapter {
public:
    /// Все ли нужные настройки загрузились?
    virtual bool isValid() const = 0;

    virtual ~ISettingsAdapter() {}
};

//---------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
