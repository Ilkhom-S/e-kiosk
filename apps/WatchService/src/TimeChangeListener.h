/* @file Класс, отслеживающий смену системного времени. */

#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QMutex>
#include <QtCore/QObject>

#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#endif

//------------------------------------------------------------------------
class TimeChangeListener : public QObject {
    Q_OBJECT

public:
    TimeChangeListener(QObject *aParent);
    virtual ~TimeChangeListener();

signals:
    /// сигнал об изменении времени (смещение в мс. примерное)
    void timeChanged(qint64 aOffset);

protected:
    void timerEvent(QTimerEvent *aEvent);

    /// Проверка и попытка примерного вычисления смещения нового времени.
    QDateTime checkTimeOffset();
    
    /// Внутренний helper - предполагает, что m_HookMutex уже захвачен
    QDateTime checkTimeOffsetLocked();

protected slots:
    /// слот для развязывания хука с signal\slot Qt
    void emitTimeChanged();

    /// очистка смещения времени
    void cleanTimeOffset();

#ifdef Q_OS_WIN
protected:
    static LRESULT CALLBACK MsgProc(int aCode, WPARAM aWParam, LPARAM aLParam);

private:
    static HHOOK m_Hook;
#endif // Q_OS_WIN

private:
    static QMutex m_HookMutex;

private:
    QDateTime m_LastCheckTime;
    qint64 m_TimeOffset;
};

//------------------------------------------------------------------------
