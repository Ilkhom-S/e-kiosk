/* @file Мьютекс-локер с возможностью ограничения синхронизации определенного потока. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QMutex>
#include <QtCore/QPair>
#include <QtCore/QRecursiveMutex>
#include <QtCore/QThread>

//--------------------------------------------------------------------------------
class MutexLocker {
    typedef QMap<QThread *, QThread *> TMatchedThreads;
    typedef QPair<QThread *, int> TLocksCounter;

    // Union to store either mutex type
    union MutexUnion {
        QMutex *mutex;
        QRecursiveMutex *recursiveMutex;
        MutexUnion() : mutex(nullptr) {}
    };

    typedef QMap<void *, TLocksCounter> TThreadsLocked;

public:
    MutexLocker(QMutex *aMutex);
    MutexLocker(QRecursiveMutex *aMutex);
    ~MutexLocker();

    /// Замапить потока с ограниченной синхронизацией на вызвавший его поток.
    static void setMatchedThread(QThread *aOwner, QThread *aMatched);

    /// Удалить данные потоке с ограниченной синхронизацией.
    static void clearMatchedThread(QThread *aOwner);

private:
    /// Рабочий мьютекс (union for both types).
    MutexUnion m_MutexUnion;

    /// Тип мьютекса (true = recursive, false = regular).
    bool m_IsRecursive;

    /// Таблица соответствия потока, вызвавшего локер, и замещающего его потока с ограниченной
    /// синхронизацией.
    static TMatchedThreads m_MatchedThreads;

    /// Потоки, залочившие мьютексы + количество локов (для рекурсивных мьютексов).
    static TThreadsLocked m_ThreadsLocked;

    // Мьютекс для защиты ресурсов.
    static QRecursiveMutex m_ResourceMutex;
};

//--------------------------------------------------------------------------------
