/* @file Интерфейс обеспечивающий взаимодействие с денежной подсистемой. */

#pragma once

// Stl
#include <QtCore/QObject>
#include <QtCore/QtGlobal>

#include <SDK/PaymentProcessor/Core/ICashAcceptorManager.h>
#include <SDK/PaymentProcessor/Core/ICashDispenserManager.h>

#include <tuple>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class IFundsService {
public:
    /// Получить интерфейс для работы с источниками денег.
    virtual ICashAcceptorManager *getAcceptor() const = 0;

    /// Получить интерфейс для работы с устройствами выдачи денег.
    virtual ICashDispenserManager *getDispenser() const = 0;

protected:
    virtual ~IFundsService() {}
};

//------------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
