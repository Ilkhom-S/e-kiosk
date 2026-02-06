/* @file Реализация задачи инкассации терминала по расписанию. */

#pragma once

#include <Common/ILogable.h>

#include <SDK/PaymentProcessor/Core/ITask.h>

namespace Ucs {

//---------------------------------------------------------------------------
class UscEncashTask : public QObject, public SDK::PaymentProcessor::ITask {
    Q_OBJECT

public:
    UscEncashTask(const QString &aName, const QString &aLogName, const QString &aParams);

    /// выполнить задачу
    virtual void execute();

    /// остановить выполнение задачи
    virtual bool cancel() { return true; };

    /// подписаться на сигнал окончания задания
    virtual bool subscribeOnComplete(QObject *aReceiver, const char *aSlot);

signals:
    void finished(const QString &aName, bool aComplete);
};

} // namespace Ucs
