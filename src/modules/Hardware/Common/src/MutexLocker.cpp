/* @file Мьютекс-локер с возможностью ограничения синхронизации определенного потока. */

// Project
#include "MutexLocker.h"

MutexLocker::TMatchedThreads MutexLocker::mMatchedThreads;
QRecursiveMutex MutexLocker::mResourceMutex;
MutexLocker::TThreadsLocked MutexLocker::mThreadsLocked;

//--------------------------------------------------------------------------------
MutexLocker::MutexLocker(QMutex *aMutex) : mIsRecursive(false) {
    mMutexUnion.mutex = aMutex;
    if (aMutex) {
        QThread *matchedThread = nullptr;

        {
            QMutexLocker locker(&mResourceMutex);

            if (!mThreadsLocked.contains(aMutex)) {
                mThreadsLocked.insert(aMutex, TLocksCounter(nullptr, 0));
            } else if (mMatchedThreads.contains(mThreadsLocked[aMutex].first)) {
                matchedThread = mMatchedThreads[mThreadsLocked[aMutex].first];
            }
        }

        QThread *currentThread = QThread::currentThread();

        if (matchedThread != currentThread) {
            mMutexUnion.mutex->lock();

            {
                QMutexLocker locker(&mResourceMutex);

                mThreadsLocked[aMutex].first = currentThread;
                mThreadsLocked[aMutex].second++;
            }
        }
    }
}

//--------------------------------------------------------------------------------
MutexLocker::MutexLocker(QRecursiveMutex *aMutex) : mIsRecursive(true) {
    mMutexUnion.recursiveMutex = aMutex;
    if (aMutex) {
        QThread *matchedThread = nullptr;

        {
            QMutexLocker locker(&mResourceMutex);

            if (!mThreadsLocked.contains(aMutex)) {
                mThreadsLocked.insert(aMutex, TLocksCounter(nullptr, 0));
            } else if (mMatchedThreads.contains(mThreadsLocked[aMutex].first)) {
                matchedThread = mMatchedThreads[mThreadsLocked[aMutex].first];
            }
        }

        QThread *currentThread = QThread::currentThread();

        if (matchedThread != currentThread) {
            mMutexUnion.recursiveMutex->lock();

            {
                QMutexLocker locker(&mResourceMutex);

                mThreadsLocked[aMutex].first = currentThread;
                mThreadsLocked[aMutex].second++;
            }
        }
    }
}

//--------------------------------------------------------------------------------
MutexLocker::~MutexLocker() {
    if (mIsRecursive) {
        if (mMutexUnion.recursiveMutex) {
            QMutexLocker locker(&mResourceMutex);

            QThread *currentThread = QThread::currentThread();

            if (mThreadsLocked.contains(mMutexUnion.recursiveMutex) &&
                (mThreadsLocked[mMutexUnion.recursiveMutex].first == currentThread)) {
                mMutexUnion.recursiveMutex->unlock();
                mThreadsLocked[mMutexUnion.recursiveMutex].second--;

                if (mThreadsLocked[mMutexUnion.recursiveMutex].second <= 0) {
                    mThreadsLocked[mMutexUnion.recursiveMutex].first = nullptr;
                }
            }
        }
    } else {
        if (mMutexUnion.mutex) {
            QMutexLocker locker(&mResourceMutex);

            QThread *currentThread = QThread::currentThread();

            if (mThreadsLocked.contains(mMutexUnion.mutex) &&
                (mThreadsLocked[mMutexUnion.mutex].first == currentThread)) {
                mMutexUnion.mutex->unlock();
                mThreadsLocked[mMutexUnion.mutex].second--;

                if (mThreadsLocked[mMutexUnion.mutex].second <= 0) {
                    mThreadsLocked[mMutexUnion.mutex].first = nullptr;
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------
void MutexLocker::setMatchedThread(QThread *aOwner, QThread *aMatched) {
    QMutexLocker locker(&mResourceMutex);

    mMatchedThreads.insert(aOwner, aMatched);
}

//--------------------------------------------------------------------------------
void MutexLocker::clearMatchedThread(QThread *aOwner) {
    QMutexLocker locker(&mResourceMutex);

    mMatchedThreads.remove(aOwner);
}

//--------------------------------------------------------------------------------
