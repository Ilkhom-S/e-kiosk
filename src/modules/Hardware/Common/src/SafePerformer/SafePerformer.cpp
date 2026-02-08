/* @file Класс для выполнения функционала без зависаний. */

#include "Hardware/Common/SafePerformer.h"

#include <QtCore/QElapsedTimer>

#include "Hardware/Common/MutexLocker.h"

SafePerformerThread::SafePerformerThread(ILog *aLog) : m_Log(aLog) {
    moveToThread(this);
}

//--------------------------------------------------------------------------------
void SafePerformerThread::onTask(const STaskData &aData) {
    LOG(m_Log, LogLevel::Debug, "Task started");

    QElapsedTimer timer;
    timer.start();

    bool result = aData.task();
    qint64 performingTime = timer.elapsed();

    if (performingTime < aData.timeout) {
        LOG(m_Log, LogLevel::Debug, QString("Task was performed for %1 ms").arg(performingTime));

        emit finished(result);
    } else {
        LOG(m_Log, LogLevel::Error, QString("Task was performed for %1 ms").arg(performingTime));
        aData.changePerformingTimeout(aData.context, aData.timeout, performingTime);

        if (aData.forwardingTask) {
            LOG(m_Log, LogLevel::Normal, "Going to forwarding task");

            aData.forwardingTask();
        }
    }
}

//--------------------------------------------------------------------------------
SafePerformer::SafePerformer(ILog *aLog) : m_Log(aLog), m_Result(false) {
    qRegisterMetaType<STaskData>("STaskData");
}

//--------------------------------------------------------------------------------
ETaskResult::Enum SafePerformer::process(const STaskData &aData) {
    if (!aData.task) {
        return ETaskResult::Invalid;
    }

    auto *workingThread = new SafePerformerThread(m_Log);
    connect(workingThread, SIGNAL(finished(bool)), SLOT(onFinished(bool)));

    moveToThread(workingThread);
    workingThread->start();

    m_Result = false;

    {
        QMutexLocker locker(&m_Guard);

        QMetaObject::invokeMethod(
            workingThread, "onTask", Qt::QueuedConnection, Q_ARG(const STaskData &, aData));

        if (!m_WaitCondition.wait(&m_Guard, aData.timeout)) {
            LOG(m_Log,
                LogLevel::Error,
                "Cannot perform task during " + QString::number(aData.timeout));
            return ETaskResult::Suspended;
        }

        workingThread->quit();
        workingThread->wait();
    }

    delete workingThread;

    return m_Result ? ETaskResult::OK : ETaskResult::Error;
}

//--------------------------------------------------------------------------------
void SafePerformer::onFinished(bool aSuccess) {
    m_Result = aSuccess;

    m_WaitCondition.wakeAll();
}

//--------------------------------------------------------------------------------
