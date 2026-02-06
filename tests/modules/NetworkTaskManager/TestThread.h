/* @file Поток для проверки сетевых запросов. */

#pragma once

#include <QtCore/QThread>

class NetworkTaskManager;

//------------------------------------------------------------------------
/// Класс для тестирования менеджера сетевых запросов.
class TestThread : public QThread {
    Q_OBJECT

public:
    TestThread(NetworkTaskManager *aManager);
    virtual ~TestThread();

    bool taskComplete() const;

protected:
    virtual void run();

private slots:
    void onTaskComplete();

private:
    NetworkTaskManager *m_manager;
    bool m_taskComplete;
};

//------------------------------------------------------------------------
