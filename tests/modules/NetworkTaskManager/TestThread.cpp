/* @file Поток для проверки сетевых запросов. */

#include "TestThread.h"

#include <QtCore/QFile>

#include "NetworkTaskManager/FileDownloadTask.h"
#include "NetworkTaskManager/NetworkTaskManager.h"
#include "TestClass.h"

TestThread::TestThread(NetworkTaskManager *aManager) : m_manager(aManager), m_taskComplete(false) {
    moveToThread(this);
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

    if (connect(&task, SIGNAL(onComplete()), SLOT(onTaskComplete())) == nullptr) {
        return;
    }

    m_manager->addTask(&task);

    QThread::exec();
}

//------------------------------------------------------------------------
void TestThread::onTaskComplete() {
    auto *task = dynamic_cast<NetworkTask *>(sender());

    m_taskComplete = (task->getError() == NetworkTask::NoError);

    quit();
}

//------------------------------------------------------------------------
bool TestThread::taskComplete() const {
    return m_taskComplete;
}

//------------------------------------------------------------------------
