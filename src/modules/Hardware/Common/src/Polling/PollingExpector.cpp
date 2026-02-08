/* @file Класс-expector для ожидания состояния. */

#include "Hardware/Common/PollingExpector.h"

#include "Hardware/Common/MutexLocker.h"

template bool PollingExpector::wait<void>(TVoidMethod aOnPoll,
                                          TBoolMethod aCondition,
                                          TBoolMethod aErrorCondition,
                                          const SWaitingData &aWaitingData);
template bool PollingExpector::wait<void>(TVoidMethod aOnPoll,
                                          TBoolMethod aCondition,
                                          TBoolMethod aErrorCondition,
                                          int aPollingInterval,
                                          int aTimeout,
                                          bool aPollingSensible);
template bool PollingExpector::wait<void>(TVoidMethod aOnPoll,
                                          TBoolMethod aCondition,
                                          const SWaitingData &aWaitingData);
template bool PollingExpector::wait<void>(TVoidMethod aOnPoll,
                                          TBoolMethod aCondition,
                                          int aPollingInterval,
                                          int aTimeout,
                                          bool aPollingSensible);
template bool PollingExpector::wait<bool>(TBoolMethod aOnPoll,
                                          TBoolMethod aCondition,
                                          int aPollingInterval,
                                          int aTimeout,
                                          bool aPollingSensible);

//--------------------------------------------------------------------------------
ExpectorWorkingThread::ExpectorWorkingThread() {
    moveToThread(this);
    m_Polling.moveToThread(this);
    m_Owner = QThread::currentThread();
    m_PollingSensible = false;

    connect(&m_Polling, SIGNAL(timeout()), SLOT(onPoll()), Qt::QueuedConnection);
}

//--------------------------------------------------------------------------------
void ExpectorWorkingThread::run() {
    MutexLocker::setMatchedThread(m_Owner, currentThread());

    QThread::exec();

    MutexLocker::clearMatchedThread(m_Owner);
}

//--------------------------------------------------------------------------------
void ExpectorWorkingThread::process(TBoolMethod aOnPoll,
                                    TBoolMethod aCondition,
                                    TBoolMethod aErrorCondition,
                                    int aPollingInterval,
                                    bool aPollingSensible) {
    m_OnPoll = aOnPoll;
    m_PollingSensible = aPollingSensible;
    m_Condition = aCondition;
    m_ErrorCondition = aErrorCondition;
    m_Polling.setInterval(aPollingInterval);

    if (!isRunning()) {
        start();
    }

    QMetaObject::invokeMethod(&m_Polling, "start", Qt::QueuedConnection);
}

//--------------------------------------------------------------------------------
void ExpectorWorkingThread::onPoll() {
    bool result = !m_OnPoll || m_OnPoll();

    if ((!result && m_PollingSensible) || (m_ErrorCondition && m_ErrorCondition())) {
        emit finished(false);

        m_Polling.stop();
    }

    if (m_Condition()) {
        emit finished(true);

        m_Polling.stop();
    }
}

//--------------------------------------------------------------------------------
PollingExpector::PollingExpector() : m_Result(false) {
    moveToThread(&m_WorkingThread);
    connect(&m_WorkingThread, SIGNAL(finished(bool)), SLOT(onFinished(bool)));
}

//--------------------------------------------------------------------------------
bool PollingExpector::wait(TBoolMethod aCondition,
                           int aPollingInterval,
                           int aTimeout,
                           bool aPollingSensible) {
    return wait(aCondition, SWaitingData(aPollingInterval, aTimeout, aPollingSensible));
}

//--------------------------------------------------------------------------------
bool PollingExpector::wait(TBoolMethod aCondition, const SWaitingData &aWaitingData) {
    return wait(TBoolMethod(), aCondition, aWaitingData);
}

//--------------------------------------------------------------------------------
template <class T>
bool PollingExpector::wait(std::function<T()> aOnPoll,
                           TBoolMethod aCondition,
                           int aPollingInterval,
                           int aTimeout,
                           bool aPollingSensible) {
    return wait(aOnPoll, aCondition, SWaitingData(aPollingInterval, aTimeout, aPollingSensible));
}

//--------------------------------------------------------------------------------
template <class T>
bool PollingExpector::wait(std::function<T()> aOnPoll,
                           TBoolMethod aCondition,
                           const SWaitingData &aWaitingData) {
    return wait(aOnPoll, aCondition, TBoolMethod(), aWaitingData);
}

//--------------------------------------------------------------------------------
template <class T>
bool PollingExpector::wait(std::function<T()> aOnPoll,
                           TBoolMethod aCondition,
                           TBoolMethod aErrorCondition,
                           int aPollingInterval,
                           int aTimeout,
                           bool aPollingSensible) {
    return wait(aOnPoll,
                aCondition,
                aErrorCondition,
                SWaitingData(aPollingInterval, aTimeout, aPollingSensible));
}

//--------------------------------------------------------------------------------
template <class T>
bool PollingExpector::wait(std::function<T()> aOnPoll,
                           TBoolMethod aCondition,
                           TBoolMethod aErrorCondition,
                           const SWaitingData &aWaitingData) {
    return wait<bool>(
        [&]() -> bool {
            aOnPoll();
            return true;
        },
        aCondition,
        aErrorCondition,
        aWaitingData);
}

//--------------------------------------------------------------------------------
template <>
bool PollingExpector::wait(std::function<bool()> aOnPoll,
                           TBoolMethod aCondition,
                           TBoolMethod aErrorCondition,
                           const SWaitingData &aWaitingData) {
    m_Result = false;

    if (!aCondition) {
        return false;
    }

    bool OK = aCondition();
    bool error = aErrorCondition && aErrorCondition();

    if (OK || error) {
        return OK && !error;
    }

    if (aOnPoll && !aOnPoll() && aWaitingData.pollingSensible) {
        return false;
    }

    OK = aCondition();
    error = aErrorCondition && aErrorCondition();

    if (OK || error) {
        return OK && !error;
    }

    {
        QMutexLocker locker(&m_Guard);

        m_WorkingThread.process(aOnPoll,
                               aCondition,
                               aErrorCondition,
                               aWaitingData.pollingInterval,
                               aWaitingData.pollingSensible);
        m_WaitCondition.wait(&m_Guard, aWaitingData.timeout);
    }

    m_WorkingThread.quit();
    m_WorkingThread.wait();

    return m_Result;
}

//--------------------------------------------------------------------------------
void PollingExpector::onFinished(bool aSuccess) {
    m_WaitCondition.wakeAll();
    m_Result = aSuccess;
}

//--------------------------------------------------------------------------------
