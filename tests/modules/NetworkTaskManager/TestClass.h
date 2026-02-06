/* @file Юнит тест для проверки менеджера сетевых запросов. */

#pragma once

#include <QtCore/QObject>

#include <Common/BasicApplication.h>

#include <NetworkTaskManager/NetworkTaskManager.h>

class NetworkTask;

//------------------------------------------------------------------------
const QUrl TestUrl = QUrl(QString("http://httpbin.org/range/1024"));
const QString TestFile = "test.bin";

//------------------------------------------------------------------------
/// Класс для тестирования менеджера сетевых запросов.
class NetworkTaskManagerTestClass : public QObject {
    Q_OBJECT

public:
    NetworkTaskManagerTestClass();
    virtual ~NetworkTaskManagerTestClass();

private slots:
    /// Тест выполнения запроса GET.
    void httpGetTest();

    /// Тест таймаута выполнения запроса.
    void httpGetTimeoutTest();

    /// Тест докачки данных.
    void httpGetRegetTest();

    /// Тест докачки с заведомо неправильным размером файла.
    void httpGetRegetFailTest();

    /// Тест скачивания файла и работоспособности сигналов класса NetworkTask.
    void httpDownloadFileAndSignalsTest();

private:
    BasicApplication m_application;
    NetworkTaskManager m_manager;
};

//------------------------------------------------------------------------
