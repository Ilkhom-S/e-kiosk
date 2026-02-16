/* @file Класс, отслеживающий активность пользователя. */

#include "TimeChangeListener.h"

#include <QtCore/QDateTime>
#include <QtCore/QMetaObject>
#include <QtCore/QMutexLocker>
#include <QtCore/QThread>
#include <QtCore/QTimer>

TimeChangeListener *gListener;

namespace CTimeChangeListener {
/// время периода проверки изменения времени
const int CheckTimerTimeout = 100; // ms
} // namespace CTimeChangeListener

#ifdef Q_OS_WIN
HHOOK TimeChangeListener::m_Hook;
#endif
QMutex TimeChangeListener::m_HookMutex;

//------------------------------------------------------------------------
TimeChangeListener::TimeChangeListener(QObject *aParent) : QObject(aParent), m_TimeOffset(0) {
    gListener = this;

#ifdef Q_OS_WIN
    m_Hook =
        ::SetWindowsHookEx(WH_GETMESSAGE, &TimeChangeListener::MsgProc, 0, ::GetCurrentThreadId());
#endif

    startTimer(CTimeChangeListener::CheckTimerTimeout);
}

//------------------------------------------------------------------------
TimeChangeListener::~TimeChangeListener() {
#ifdef Q_OS_WIN
    ::UnhookWindowsHookEx(m_Hook);
#endif
}

//------------------------------------------------------------------------
void TimeChangeListener::timerEvent(QTimerEvent *aEvent) {
    QMutexLocker locker(&m_HookMutex);

#ifndef Q_OS_WIN
    m_LastCheckTime = checkTimeOffsetLocked();
#else
    m_LastCheckTime = QDateTime::currentDateTime();
#endif
}

//------------------------------------------------------------------------
QDateTime TimeChangeListener::checkTimeOffset() {
    QMutexLocker locker(&m_HookMutex);
    return checkTimeOffsetLocked();
}

//------------------------------------------------------------------------
QDateTime TimeChangeListener::checkTimeOffsetLocked() {
    // Предполагается, что m_HookMutex уже захвачен вызывающей стороной
    QDateTime currentTime = QDateTime::currentDateTime();

    if (m_TimeOffset == 0) {
        qint64 offset = m_LastCheckTime.msecsTo(currentTime);

        if (qAbs(offset) > CTimeChangeListener::CheckTimerTimeout) {
            // время поменяли!
            m_TimeOffset = offset;

            QMetaObject::invokeMethod(this, "emitTimeChanged", Qt::QueuedConnection);
        }
    }

    return currentTime;
}

//------------------------------------------------------------------------
void TimeChangeListener::emitTimeChanged() {
    if (m_TimeOffset != 0) {
        QMutexLocker locker(&m_HookMutex);

        emit timeChanged(m_TimeOffset);

        // Use new Qt5/6 compatible signal/slot syntax with lambda
        QTimer::singleShot(2000, this, [this]() { cleanTimeOffset(); });
    }
}

//------------------------------------------------------------------------
void TimeChangeListener::cleanTimeOffset() {
    m_TimeOffset = 0;
}

#ifdef Q_OS_WIN
//------------------------------------------------------------------------
LRESULT CALLBACK TimeChangeListener::MsgProc(int aCode, WPARAM aWParam, LPARAM aLParam) {
    if (aCode == HC_ACTION && ((MSG *)aLParam)->message == WM_TIMECHANGE) {
        gListener->checkTimeOffset();
    }

    return ::CallNextHookEx(m_Hook, aCode, aWParam, aLParam);
}
#endif // Q_OS_WIN
