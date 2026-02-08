/* @file Мьютекс-локер с возможностью ограничения синхронизации определенного потока. */

#include "MutexLocker.h"

MutexLocker::TMatchedThreads MutexLocker::m_MatchedThreads;
QRecursiveMutex MutexLocker::m_ResourceMutex;
MutexLocker::TThreadsLocked MutexLocker::m_ThreadsLocked;

//--------------------------------------------------------------------------------
MutexLocker::MutexLocker(QMutex *aMutex) : m_IsRecursive(false) {
    m_MutexUnion.mutex = aMutex;
    if (aMutex) {
        QThread *matchedThread = nullptr;

        {
            QMutexLocker locker(&m_ResourceMutex);

            if (!m_ThreadsLocked.contains(aMutex)) {
                m_ThreadsLocked.insert(aMutex, TLocksCounter(nullptr, 0));
            } else if (m_MatchedThreads.contains(m_ThreadsLocked[aMutex].first)) {
                matchedThread = m_MatchedThreads[m_ThreadsLocked[aMutex].first];
            }
        }

        QThread *currentThread = QThread::currentThread();

        if (matchedThread != currentThread) {
            m_MutexUnion.mutex->lock();

            {
                QMutexLocker locker(&m_ResourceMutex);

                m_ThreadsLocked[aMutex].first = currentThread;
                m_ThreadsLocked[aMutex].second++;
            }
        }
    }
}

//--------------------------------------------------------------------------------
MutexLocker::MutexLocker(QRecursiveMutex *aMutex) : m_IsRecursive(true) {
    m_MutexUnion.recursiveMutex = aMutex;
    if (aMutex) {
        QThread *matchedThread = nullptr;

        {
            QMutexLocker locker(&m_ResourceMutex);

            if (!m_ThreadsLocked.contains(aMutex)) {
                m_ThreadsLocked.insert(aMutex, TLocksCounter(nullptr, 0));
            } else if (m_MatchedThreads.contains(m_ThreadsLocked[aMutex].first)) {
                matchedThread = m_MatchedThreads[m_ThreadsLocked[aMutex].first];
            }
        }

        QThread *currentThread = QThread::currentThread();

        if (matchedThread != currentThread) {
            m_MutexUnion.recursiveMutex->lock();

            {
                QMutexLocker locker(&m_ResourceMutex);

                m_ThreadsLocked[aMutex].first = currentThread;
                m_ThreadsLocked[aMutex].second++;
            }
        }
    }
}

//--------------------------------------------------------------------------------
MutexLocker::~MutexLocker() {
    if (m_IsRecursive) {
        if (m_MutexUnion.recursiveMutex) {
            QMutexLocker locker(&m_ResourceMutex);

            QThread *currentThread = QThread::currentThread();

            if (m_ThreadsLocked.contains(m_MutexUnion.recursiveMutex) &&
                (m_ThreadsLocked[m_MutexUnion.recursiveMutex].first == currentThread)) {
                m_MutexUnion.recursiveMutex->unlock();
                m_ThreadsLocked[m_MutexUnion.recursiveMutex].second--;

                if (m_ThreadsLocked[m_MutexUnion.recursiveMutex].second <= 0) {
                    m_ThreadsLocked[m_MutexUnion.recursiveMutex].first = nullptr;
                }
            }
        }
    } else {
        if (m_MutexUnion.mutex) {
            QMutexLocker locker(&m_ResourceMutex);

            QThread *currentThread = QThread::currentThread();

            if (m_ThreadsLocked.contains(m_MutexUnion.mutex) &&
                (m_ThreadsLocked[m_MutexUnion.mutex].first == currentThread)) {
                m_MutexUnion.mutex->unlock();
                m_ThreadsLocked[m_MutexUnion.mutex].second--;

                if (m_ThreadsLocked[m_MutexUnion.mutex].second <= 0) {
                    m_ThreadsLocked[m_MutexUnion.mutex].first = nullptr;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------
void MutexLocker::setMatchedThread(QThread *aOwner, QThread *aMatched) {
    QMutexLocker locker(&m_ResourceMutex);

    m_MatchedThreads.insert(aOwner, aMatched);
}

//--------------------------------------------------------------------------------
void MutexLocker::clearMatchedThread(QThread *aOwner) {
    QMutexLocker locker(&m_ResourceMutex);

    m_MatchedThreads.remove(aOwner);
}

//--------------------------------------------------------------------------------
