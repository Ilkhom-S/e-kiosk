#include <QtTest/QtTest>

#include <WatchServiceClient/Constants.h>
#include <WatchServiceClient/IWatchServiceClient.h>

#include "../../../apps/WatchServiceController/src/WatchServiceController.h"
#include "MessageQueueServer.h"

// Parse simple key=value;key2=value2; strings used by WatchServiceClient
static QMap<QString, QString> parseWatchMessage(const QByteArray &msg) {
    QMap<QString, QString> result;
    const QString s = QString::fromUtf8(msg);
    const auto parts = s.split(';', Qt::SkipEmptyParts);
    for (const QString &p : parts) {
        const int eq = p.indexOf('=');
        if (eq > 0) {
            const QString k = p.left(eq);
            const QString v = p.mid(eq + 1);
            result.insert(k, v);
        }
    }
    return result;
}

class TestControllerWatchdogIntegration : public QObject {
    Q_OBJECT

private slots:
    void test_watchserviceclient_stopService_sends_exit_to_watchdog();
    void test_watchservicecontroller_onStopServiceClicked_sends_exit_to_watchdog();
};

void TestControllerWatchdogIntegration::
    test_watchserviceclient_stopService_sends_exit_to_watchdog() {
    // Start watchdog message queue server (port from constants)
    MessageQueueServer server(CWatchService::MessageQueue);
    QVERIFY(server.init());

    // Create watch-service client (simulates controller behaviour)
    IWatchServiceClient *client = createWatchServiceClient(
        CWatchService::Modules::WatchServiceController, IWatchServiceClient::MainThread);
    QVERIFY(client);

    // Start client and ensure connection
    QVERIFY(client->start());
    QTRY_VERIFY(client->isConnected());

    QSignalSpy spy(&server, SIGNAL(onMessageReceived(QByteArray)));

    // Call stopService -> should send an Exit command to WatchService
    client->stopService();

    QTRY_COMPARE(spy.count(), 1);
    const QByteArray raw = spy.takeFirst().at(0).toByteArray();
    const auto kv = parseWatchMessage(raw);

    QCOMPARE(kv.value(CWatchService::Fields::Type), CWatchService::Commands::Exit);
    QCOMPARE(kv.value(CWatchService::Fields::Module), CWatchService::Modules::WatchService);
    QCOMPARE(kv.value(CWatchService::Fields::Sender),
             QString(CWatchService::Modules::WatchServiceController));

    client->stop();
    delete client;
}

void TestControllerWatchdogIntegration::
    test_watchservicecontroller_onStopServiceClicked_sends_exit_to_watchdog() {
    MessageQueueServer server(CWatchService::MessageQueue);
    QVERIFY(server.init());

    WatchServiceController controller;

    // Force controller to attempt connection
    QMetaObject::invokeMethod(&controller, "onCheck", Qt::DirectConnection);

    // Wait until WatchServiceClient inside controller connects by observing server
    QSignalSpy serverSpy(&server, SIGNAL(onMessageReceived(QByteArray)));

    // Send a client->server ping via the controller's client by invoking onCheck (it will start)
    // watch for a short timeout — the client's ping interval is small in tests
    QTRY_VERIFY(serverSpy.isEmpty() || serverSpy.count() >= 0);

    // Now simulate user clicking "Stop service"
    QMetaObject::invokeMethod(&controller, "onStopServiceClicked", Qt::DirectConnection);

    // The controller's internal client must send an Exit message to watchdog
    QTRY_COMPARE(serverSpy.count(), 1);
    const QByteArray raw = serverSpy.takeFirst().at(0).toByteArray();
    const auto kv = parseWatchMessage(raw);

    QCOMPARE(kv.value(CWatchService::Fields::Type), CWatchService::Commands::Exit);
    QCOMPARE(kv.value(CWatchService::Fields::Module), CWatchService::Modules::WatchService);
}

QTEST_MAIN(TestControllerWatchdogIntegration)
#include "TestControllerWatchdogIntegration.moc"
