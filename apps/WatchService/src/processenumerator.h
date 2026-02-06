/* @file Реализация перечислителя процессов в системе. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QString>

//----------------------------------------------------------------------------
class ProcessEnumerator {
public:
    typedef quint32 PID;

    struct ProcessInfo {
        PID pid;
        QString path;
    };

    typedef QMap<PID, ProcessInfo>::const_iterator const_iterator;

public:
    ProcessEnumerator();

    const_iterator begin() const;
    const_iterator end() const;

    /// Остановить указанный процесс
    bool kill(PID aPid, quint32 &aErrorCode) const;

protected:
    /// Выполняет перечисление процессов (заполняет mList)
    void enumerate();

protected:
    QMap<PID, ProcessInfo> mProcesses;

    /// Остановка процесса грубым способом
    bool killInternal(PID aPid, quint32 &aErrorCode) const;
};

//----------------------------------------------------------------------------
