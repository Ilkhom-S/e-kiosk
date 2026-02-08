/* @file Интерфейс сервиса, обеспечивающего запуск задач в определенное время. */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>

#include <Common/ILog.h>

#include <SDK/PaymentProcessor/Core/ITask.h>

#include <boost/function.hpp>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class ISchedulerService {
protected:
    virtual ~ISchedulerService() {}

public:
    typedef boost::function<SDK::PaymentProcessor::ITask *(
        const QString &aName, const QString &aLogName, const QString &aParams)>
        TTaskCreator;

    /// зарегистрировать тип задачи в фабрике классов
    template <class C> void registerTaskType(const QString &aType) {
        auto taskCreator = [](const QString &aName, const QString &aLogName, const QString &aParams)
            -> SDK::PaymentProcessor::ITask * { return new C(aName, aLogName, aParams); };

        if (!m_Factory.contains(aType)) {
            m_Factory[aType] = TTaskCreator(taskCreator);
        }
    }

    void registerTaskType(const QString &aName, SDK::PaymentProcessor::ITask *aTask) {
        if (!m_ExternalTasks.contains(aName)) {
            m_ExternalTasks[aName] = aTask;
        }
    }

protected:
    QMap<QString, TTaskCreator> m_Factory;
    QMap<QString, SDK::PaymentProcessor::ITask *> m_ExternalTasks;
};

//------------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
