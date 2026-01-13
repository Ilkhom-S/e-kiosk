/* @file Юнит тест для проверки менеджера сетевых запросов. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtCore/QCoreApplication>
#include <QtCore/QCryptographicHash>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkProxy>
#include <QtTest/QtTest>
#include <Common/QtHeadersEnd.h>

// System
#include <NetworkTaskManager/DataStream.h>
#include <NetworkTaskManager/HashVerifier.h>
#include <NetworkTaskManager/NetworkTask.h>

// Project
#include "TestClass.h"
#include "TestThread.h"

NetworkTaskManagerTestClass::NetworkTaskManagerTestClass()
    : m_application("TestUnit", "1.0", 0, nullptr) {}

//------------------------------------------------------------------------
NetworkTaskManagerTestClass::~NetworkTaskManagerTestClass() {}

//------------------------------------------------------------------------
void NetworkTaskManagerTestClass::httpGetTest() {
    QScopedPointer<NetworkTask> task(new NetworkTask());

    task->setUrl(TestUrl);
    task->setDataStream(new DataStream(new QBuffer()));

    m_manager.addTask(task.data());

    task->waitForFinished();

    QVERIFY(task->getError() == NetworkTask::NoError);
}

//------------------------------------------------------------------------
void NetworkTaskManagerTestClass::httpGetTimeoutTest() {
    QScopedPointer<NetworkTask> task(new NetworkTask());

    task->setUrl(TestUrl);
    task->setDataStream(new DataStream(new QBuffer()));
    task->setTimeout(1);

    m_manager.addTask(task.data());

    task->waitForFinished();

    QVERIFY(task->getError() == NetworkTask::Timeout);
}

//------------------------------------------------------------------------
void NetworkTaskManagerTestClass::httpGetRegetTest() {
    QScopedPointer<NetworkTask> task(new NetworkTask());

    task->setUrl(TestUrl);
    task->setDataStream(new DataStream(new QBuffer()));

    m_manager.addTask(task.data());

    task->waitForFinished();

    QVERIFY(task->getError() == NetworkTask::NoError);

    // Прочитанные данные
    QByteArray data = task->getDataStream()->readAll();

    // Получаем md5-хеш прочитанных данных
    QString originalMD5 =
        QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();

    // Отрезаем половину прочитанных данных
    data.resize(data.size() / 2);

    QBuffer *buffer = new QBuffer();
    buffer->setData(data);

    task->setDataStream(new DataStream(buffer));

    // Очищаем полученные http заголовки
    // task->getHeaders().clear(); // TODO: implement if needed

    // Добавляем флаг "дозакачки"
    task->setFlags(NetworkTask::Continue);

    // Устанавливаем верификатор данных
    task->setVerifier(new Md5Verifier(originalMD5));

    m_manager.addTask(task.get());

    task->waitForFinished();

    QVERIFY(task->getError() == NetworkTask::NoError);
}

//------------------------------------------------------------------------
void NetworkTaskManagerTestClass::httpGetRegetFailTest() {
    QScopedPointer<NetworkTask> task(new NetworkTask());

    task->setUrl(TestUrl);
    task->setDataStream(new DataStream(new QBuffer()));

    m_manager.addTask(task.data());

    task->waitForFinished();

    QVERIFY(task->getError() == NetworkTask::NoError);

    // Не изменяем размер данных, получаем неправильный range

    // Добавляем флаг "дозакачки"
    task->setFlags(NetworkTask::Continue);

    // Очищаем полученные http заголовки
    // task->getHeaders().clear(); // TODO: implement if needed

    m_manager.addTask(task.data());

    task->waitForFinished();

    QVERIFY(task->getError() == NetworkTask::BadTask);
}

//------------------------------------------------------------------------
void NetworkTaskManagerTestClass::httpDownloadFileAndSignalsTest() {
    TestThread thread(&m_manager);

    thread.start();
    thread.wait(20000);

    QVERIFY(thread.taskComplete());
}

//------------------------------------------------------------------------
