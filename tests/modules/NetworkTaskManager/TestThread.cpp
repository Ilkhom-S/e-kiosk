/* @file Поток для проверки сетевых запросов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFile>
#include <Common/QtHeadersEnd.h>

// System
#include "NetworkTaskManager/FileDownloadTask.h"
#include "NetworkTaskManager/NetworkTaskManager.h"

// Project
#include "TestClass.h"
#include "TestThread.h"

TestThread::TestThread(NetworkTaskManager *aManager) {
    moveToThread(this);

    m_manager = aManager;
    m_taskComplete = false;
}

//------------------------------------------------------------------------
TestThread::~TestThread() {
    quit();
    wait();
}

//------------------------------------------------------------------------
void TestThread::run() {
    QString filePath = BasicApplication::getInstance()->getWorkingDirectory() + "/" + TestFile;

    // Remove existing file to ensure clean download
    QFile::remove(filePath);

    FileDownloadTask task(TestUrl, filePath);

    if (!connect(&task, SIGNAL(onComplete()), SLOT(onTaskComplete()))) {
        return;
    }

    m_manager->addTask(&task);

    QThread::exec();
}

//------------------------------------------------------------------------
void TestThread::onTaskComplete() {
    NetworkTask *task = dynamic_cast<NetworkTask *>(sender());

    m_taskComplete = (task->getError() == NetworkTask::NoError);

    quit();
}

//------------------------------------------------------------------------
bool TestThread::taskComplete() const {
    return m_taskComplete;
}

//------------------------------------------------------------------------
