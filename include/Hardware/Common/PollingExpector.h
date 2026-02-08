/* @file Класс-expector для ожидания состояния. */

#pragma once

#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>

#include "Hardware/Common/FunctionTypes.h"
#include "Hardware/Common/WaitingData.h"

//--------------------------------------------------------------------------------
/// Рабочий поток для класса-ожидателя.
class ExpectorWorkingThread : public QThread {
    Q_OBJECT

public:
    ExpectorWorkingThread();
    void process(TBoolMethod aOnPoll,
                 TBoolMethod aCondition,
                 TBoolMethod aErrorCondition,
                 int aPollingInterval,
                 bool aPollingSensible);

public slots:
    void onPoll(); /// Опрос состояния.

signals:
    void finished(bool aSuccess); /// Завершено.

private:
    virtual void run(); /// QThread: Рабочая процедура Qt'шной нити.

    QThread *m_Owner;             /// Указатель на вызвавший поток.
    QTimer m_Polling;             /// Таймер для поллинга.
    TBoolMethod m_OnPoll;         /// Функтор поллинга.
    TBoolMethod m_Condition;      /// Функтор условия ожидания.
    TBoolMethod m_ErrorCondition; /// Функтор условия ошибки.
    bool m_PollingSensible;       /// В условие ожидания включен контроль результата поллинга.
};

//--------------------------------------------------------------------------------
/// Класс-ожидатель.
class PollingExpector : public QObject {
    Q_OBJECT

public:
    PollingExpector();

    /// Ожидание состояния или выполнения полла.
    bool
    wait(TBoolMethod aCondition, int aPollingInterval, int aTimeout, bool aPollingSensible = false);
    bool wait(TBoolMethod aCondition, const SWaitingData &aWaitingData);

    /// Ожидание состояния.
    template <class T>
    bool wait(std::function<T()> aOnPoll,
              TBoolMethod aCondition,
              int aPollingInterval,
              int aTimeout,
              bool aPollingSensible = false);
    template <class T>
    bool wait(std::function<T()> aOnPoll, TBoolMethod aCondition, const SWaitingData &aWaitingData);

    /// Ожидание состояния или ошибки.
    template <class T>
    bool wait(std::function<T()> aOnPoll,
              TBoolMethod aCondition,
              TBoolMethod aErrorCondition,
              int aPollingInterval,
              int aTimeout,
              bool aPollingSensible = false);
    template <class T>
    bool wait(std::function<T()> aOnPoll,
              TBoolMethod aCondition,
              TBoolMethod aErrorCondition,
              const SWaitingData &aWaitingData);

public slots:
    void onFinished(bool aSuccess); /// Завершено.

private:
    bool m_Result;                         /// Результат ожидания.
    QMutex m_Guard;                        /// Сторож для wait condition.
    QWaitCondition m_WaitCondition;        /// Wait condition для таймаута ожидания.
    ExpectorWorkingThread m_WorkingThread; /// Рабочий поток.
};

//--------------------------------------------------------------------------------
