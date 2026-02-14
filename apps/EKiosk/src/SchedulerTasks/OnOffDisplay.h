/* @file Реализация задачи включения энергосберегающего режима. */

#pragma once

// Qt
#include <QtCore/QTime>

#include <Common/QtHeadersBegin.h>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ITask.h>

// Модули
#include <Common/ILogable.h>

//---------------------------------------------------------------------------
class OnOffDisplay : public QObject, public SDK::PaymentProcessor::ITask, public ILogable {
    Q_OBJECT

public:
    /// Конструктор задачи управления энергосбережением
    OnOffDisplay(const QString &aName, const QString &aLogName, const QString &aParams);
    virtual ~OnOffDisplay();

    /// Выполнить включение/отключение энергосберегающего режима
    virtual void execute();

    /// остановить выполнение задачи
    virtual bool cancel() { return true; }

    /// подписаться на сигнал окончания задания
    virtual bool subscribeOnComplete(QObject *aReceiver, const char *aSlot);

protected:
    bool m_Enable; /// Включить (true) или отключить (false) энергосбережение
    QTime m_From;  /// Время начала действия режима
    QTime m_Till;  /// Время окончания действия режима
    enum {
        Standby,
        ScreenSaver,
        Shutdown
    } m_Type; /// Тип действия: ждущий режим, скринсейвер или выключение

signals:
    void finished(const QString &aName, bool aComplete);
};
