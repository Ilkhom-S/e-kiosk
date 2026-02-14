/* @file Реализация задачи ротации журнальных файлов. */

#pragma once

// SDK
#include <SDK/PaymentProcessor/Core/ITask.h>

// Модули
#include <Common/ILogable.h>

//---------------------------------------------------------------------------
class LogRotate : public QObject, public SDK::PaymentProcessor::ITask {
    Q_OBJECT

public:
    /// Конструктор задачи ротации логов
    LogRotate(const QString &aName, const QString &aLogName, const QString &aParams);

    /// Выполнить задачу ротации всех журнальных файлов
    virtual void execute();

    /// остановить выполнение задачи
    virtual bool cancel() { return true; };

    /// подписаться на сигнал окончания задания
    virtual bool subscribeOnComplete(QObject *aReceiver, const char *aSlot);

signals:
    void finished(const QString &aName, bool aComplete);
};
