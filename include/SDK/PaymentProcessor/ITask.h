/* @file Интерфейс задачи для менеджера задач. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QVariant>

//------------------------------------------------------------------------------
namespace SDK {

//------------------------------------------------------------------------------
namespace PaymentProcessor {

/// Контекст задачи.
namespace TaskContext {
const int LastActivation = 0;
const int CurrentTimestamp = 1;

const int UserProperty = 100;
} // namespace TaskContext

/// Интерфейс задачи для планировщика.
class ITask {
public:
    /// Тип контекста задачи.
    typedef QMap<int, QVariant> TContext;

    /// Деструктор.
    virtual ~ITask() {};

    /// Предикат возвращает true, если задача может быть выполнена в данный момент.
    virtual bool isReady(TContext &aContext) = 0;

    /// Возвращает true, если задача должна выполняться в отдельном потоке.
    virtual bool isThread() const = 0;

    /// Рабочая процедура задачи.
    virtual void run() = 0;
};

//------------------------------------------------------------------------------
} // namespace PaymentProcessor

//------------------------------------------------------------------------------
} // namespace SDK

//------------------------------------------------------------------------------
