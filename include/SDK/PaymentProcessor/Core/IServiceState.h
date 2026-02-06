/* @file Интерфейс для получения состояния сервиса. */

#pragma once

#include <QtCore/QString>

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
class IServiceState {
public:
    virtual ~IServiceState() {}

public:
    virtual QString getState() const = 0;
};

//---------------------------------------------------------------------------

} // namespace PaymentProcessor
} // namespace SDK
