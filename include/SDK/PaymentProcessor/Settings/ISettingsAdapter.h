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
extern const char TerminalAdapter[];
extern const char DealerAdapter[];
extern const char UserAdapter[];
extern const char Directory[];
extern const char Extensions[];
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
