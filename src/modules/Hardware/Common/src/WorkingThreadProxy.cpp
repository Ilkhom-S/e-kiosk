/* @file Прослойка для вызова функционала в рабочем потоке. */

#include "WorkingThreadProxy.h"

#include <QtCore/QMetaType>
#include <QtCore/QThread>

//-------------------------------------------------------------------------------
template int WorkingThreadProxy::invokeMethod<int>(std::function<int()> aMethod);
template double WorkingThreadProxy::invokeMethod<double>(std::function<double()> aMethod);
template bool WorkingThreadProxy::invokeMethod<bool>(std::function<bool()> aMethod);
template QString WorkingThreadProxy::invokeMethod<QString>(std::function<QString()> aMethod);

//-------------------------------------------------------------------------------
WorkingThreadProxy::WorkingThreadProxy(QThread *aWorkingThread) : m_WorkingThread(aWorkingThread) {
    if (m_WorkingThread) {
        moveToThread(m_WorkingThread);
    }

    connect(this,
            SIGNAL(invoke(TVoidMethod)),
            SLOT(onInvoke(TVoidMethod)),
            Qt::BlockingQueuedConnection);
    connect(this,
            SIGNAL(invoke(TBoolMethod, bool *)),
            SLOT(onInvoke(TBoolMethod, bool *)),
            Qt::BlockingQueuedConnection);
    connect(this,
            SIGNAL(invoke(TIntMethod, int *)),
            SLOT(onInvoke(TIntMethod, int *)),
            Qt::BlockingQueuedConnection);
    connect(this,
            SIGNAL(invoke(TDoubleMethod, double *)),
            SLOT(onInvoke(TDoubleMethod, double *)),
            Qt::BlockingQueuedConnection);
    connect(this,
            SIGNAL(invoke(TStringMethod, QString *)),
            SLOT(onInvoke(TStringMethod, QString *)),
            Qt::BlockingQueuedConnection);
}

//--------------------------------------------------------------------------------
template <class T> T WorkingThreadProxy::invokeMethod(std::function<T()> aMethod) {
    checkThreadStarted();

    T result;

    isWorkingThread() ? onInvoke(aMethod, &result) : emit invoke(aMethod, &result);

    return result;
}

//--------------------------------------------------------------------------------
template <> void WorkingThreadProxy::invokeMethod(TVoidMethod aMethod) {
    checkThreadStarted();

    isWorkingThread() ? onInvoke(aMethod) : emit invoke(aMethod);
}

//--------------------------------------------------------------------------------
void WorkingThreadProxy::onInvoke(const TVoidMethod &aMethod) {
    aMethod();
}

//--------------------------------------------------------------------------------
void WorkingThreadProxy::onInvoke(const TBoolMethod &aMethod, bool *aResult) {
    *aResult = aMethod();
}

//--------------------------------------------------------------------------------
void WorkingThreadProxy::onInvoke(const TIntMethod &aMethod, int *aResult) {
    *aResult = aMethod();
}

//--------------------------------------------------------------------------------
void WorkingThreadProxy::onInvoke(const TDoubleMethod &aMethod, double *aResult) {
    *aResult = aMethod();
}

//--------------------------------------------------------------------------------
void WorkingThreadProxy::onInvoke(const TStringMethod &aMethod, QString *aResult) {
    *aResult = aMethod();
}

//--------------------------------------------------------------------------------
bool WorkingThreadProxy::isWorkingThread() {
    return (m_WorkingThread == nullptr) || (m_WorkingThread == QThread::currentThread());
}

//--------------------------------------------------------------------------------
void WorkingThreadProxy::checkThreadStarted() {
    if (m_WorkingThread) {
        if (!m_WorkingThread->isRunning()) {
            connect(m_WorkingThread,
                    SIGNAL(started()),
                    this,
                    SLOT(checkThreadStarted()),
                    Qt::UniqueConnection);
            m_WorkingThread->start();

            QMutexLocker locker(&m_StartMutex);

            m_StartCondition.wait(&m_StartMutex);
        }

        m_StartCondition.wakeAll();
    }
}

//--------------------------------------------------------------------------------
