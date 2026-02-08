/* @file База для задачи планировщика. */

#pragma once

#include <QtCore/QDateTime>

#include <boost/function.hpp>

// Sdk
#include "ITask.h"

//------------------------------------------------------------------------------
namespace SDK {

//------------------------------------------------------------------------------
namespace PaymentProcessor {

/// База задачи для планировщика.
class CommonTask : public ITask {
public:
    /// Тип условия выполнения задачи.
    typedef boost::function<bool(const ITask::TContext &)> TCondition;
    /// Тип метода выполнения задачи.
    typedef boost::function<void()> TMethod;

    /// Конструктор.
    CommonTask(TCondition aCondition, TMethod aMethod, bool aIsThread = false)
        : m_Condition(aCondition), m_Method(aMethod), m_IsThread(aIsThread) {}

    /// Деструктор.
    virtual ~CommonTask() {}

    /// ITask: Предикат возвращает true, если задача может быть выполнена в данный момент.
    virtual bool isReady(TContext &aContext) {
        aContext[TaskContext::LastActivation] = m_LastActivation;

        bool result = m_Condition(aContext);

        aContext.remove(TaskContext::LastActivation);

        return result;
    }

    /// ITask: Возвращает true, если задача должна выполняться в отдельном потоке.
    virtual bool isThread() const { return m_IsThread; }

    /// ITask: Рабочая процедура задачи.
    virtual void run() {
        m_LastActivation = QDateTime::currentDateTime();

        m_Method();
    }

private:
    TCondition m_Condition;
    TMethod m_Method;
    bool m_IsThread;

    QDateTime m_LastActivation;
};

//------------------------------------------------------------------------------
} // namespace PaymentProcessor

//------------------------------------------------------------------------------
} // namespace SDK
