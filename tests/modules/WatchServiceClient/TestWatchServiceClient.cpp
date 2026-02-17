#define UNIT_TEST_WATCHSERVICECLIENT
#include <QDebug>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include <WatchServiceClient/Constants.h>
#include <string>

#include "WatchServiceClient.h"

static const int test_watch_service_client_startup = []() {
    qDebug() << "TestWatchServiceClient: startup";
    return 0;
}();

class TestableWatchServiceClient : public WatchServiceClient {
public:
    TestableWatchServiceClient(const QString &name)
        : WatchServiceClient(name, IWatchServiceClient::MainThread) {}

    // Expose protected slot for direct testing
    void callOnMessage(const QByteArray &msg) { onMessageReceived(msg); }
};

// Minimal slot receivers for subscribeOn* tests
class CommandReceivedReceiver : public QObject {
    Q_OBJECT
public slots:
    void onCommandReceived(const QString &, const QString &, const QString &, const QStringList &) {
    }
};

class CloseCommandReceivedReceiver : public QObject {
    Q_OBJECT
public slots:
    void onCloseCommandReceived() {}
};

class DisconnectedReceiver : public QObject {
    Q_OBJECT
public slots:
    void onDisconnected() {}
};

class ModuleClosedReceiver : public QObject {
    Q_OBJECT
public slots:
    void onModuleClosed(const QString &) {}
};

class TestWatchServiceClient : public QObject {
    Q_OBJECT

private slots:
    void testCloseCommandEmitted();
    void testModuleClosedEmitted();
    void testGenericCommandEmitted();
    void testTargetFiltering();
    void testStartStop();
    void testIsConnected();
    void testExecute();
    void testStopService();
    void testRestartService();
    void testRebootMachine();
    void testShutdownMachine();
    void testStartModule();
    void testCloseModule();
    void testCloseModules();
    void testShowSplashScreen();
    void testHideSplashScreen();
    void testSetState();
    void testResetState();
    void testSubscribeOnCommandReceived();
    void testSubscribeOnCloseCommandReceived();
    void testSubscribeOnDisconnected();
    void testSubscribeOnModuleClosed();
};

void TestWatchServiceClient::testCloseCommandEmitted() {
    TestableWatchServiceClient client("client1");

    QSignalSpy spy(&client, SIGNAL(onCloseCommandReceived()));

    QByteArray msg = QString("%1=%2;%3=%4;")
                         .arg(CWatchService::Fields::Sender)
                         .arg(CWatchService::Name)
                         .arg(CWatchService::Fields::Type)
                         .arg(CWatchService::Commands::Close)
                         .toUtf8();

    qDebug() << "Test message:" << msg;
    client.callOnMessage(msg);
    qDebug() << "spy.count() after callOnMessage:" << spy.count();
    qDebug() << "lastSender:" << QString::fromStdString(g_lastSender)
             << ", lastType:" << QString::fromStdString(g_lastType)
             << ", lastTarget:" << QString::fromStdString(g_lastTarget);

    if (spy.count() != 1) {
        QFAIL(qPrintable(QString("spy.count()=%1, lastSender='%2', lastType='%3', lastTarget='%4'")
                             .arg(spy.count())
                             .arg(QString::fromStdString(g_lastSender))
                             .arg(QString::fromStdString(g_lastType))
                             .arg(QString::fromStdString(g_lastTarget))));
    }
}

void TestWatchServiceClient::testModuleClosedEmitted() {
    TestableWatchServiceClient client("client1");

    QSignalSpy spy(&client, SIGNAL(onModuleClosed(const QString &)));

    QByteArray msg = QString("%1=%2;%3=%4;")
                         .arg(CWatchService::Fields::Sender)
                         .arg("some_module")
                         .arg(CWatchService::Fields::Type)
                         .arg(CWatchService::Notification::ModuleClosed)
                         .toUtf8();

    client.callOnMessage(msg);

    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), QString("some_module"));
}

void TestWatchServiceClient::testGenericCommandEmitted() {
    TestableWatchServiceClient client("client1");

    QSignalSpy spy(&client,
                   SIGNAL(onCommandReceived(
                       const QString &, const QString &, const QString &, const QStringList &)));

    QByteArray msg = QString("%1=%2;%3=%4;%5=%6;extra;")
                         .arg(CWatchService::Fields::Sender)
                         .arg("senderX")
                         .arg(CWatchService::Fields::Type)
                         .arg("cmd")
                         .arg(CWatchService::Fields::Params)
                         .arg("p1")
                         .toUtf8();

    client.callOnMessage(msg);

    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), QString("senderX"));
    QCOMPARE(args.at(1).toString(), QString("")); // target empty
    QCOMPARE(args.at(2).toString(), QString("cmd"));
    QStringList tail = args.at(3).toStringList();
    QVERIFY(tail.contains(QString("extra")));
}

void TestWatchServiceClient::testTargetFiltering() {
    TestableWatchServiceClient client("client1");

    QSignalSpy spy(&client,
                   SIGNAL(onCommandReceived(
                       const QString &, const QString &, const QString &, const QStringList &)));

    QByteArray msg = QString("%1=%2;%3=%4;%5=%6;")
                         .arg(CWatchService::Fields::Sender)
                         .arg("senderX")
                         .arg(CWatchService::Fields::Type)
                         .arg("cmd")
                         .arg(CWatchService::Fields::Target)
                         .arg("otherClient")
                         .toUtf8();

    client.callOnMessage(msg);

    QCOMPARE(spy.count(), 0);
}

void TestWatchServiceClient::testStartStop() {
    TestableWatchServiceClient client("client1");
    QVERIFY(!client.isRunning());
    QVERIFY(!client.isConnected());
    // start() will not connect (mock returns nullptr), but should not crash
    QVERIFY(!client.start());
    client.stop();
    QVERIFY(!client.isRunning());
}

void TestWatchServiceClient::testIsConnected() {
    TestableWatchServiceClient client("client1");
    QVERIFY(!client.isConnected());
}

void TestWatchServiceClient::testExecute() {
    TestableWatchServiceClient client("client1");
    // Should not crash, even if not connected
    client.execute("cmd", "mod", "params");
}

void TestWatchServiceClient::testStopService() {
    TestableWatchServiceClient client("client1");
    client.stopService();
}

void TestWatchServiceClient::testRestartService() {
    TestableWatchServiceClient client("client1");
    client.restartService(QStringList() << "p1" << "p2");
}

void TestWatchServiceClient::testRebootMachine() {
    TestableWatchServiceClient client("client1");
    client.rebootMachine();
}

void TestWatchServiceClient::testShutdownMachine() {
    TestableWatchServiceClient client("client1");
    client.shutdownMachine();
}

void TestWatchServiceClient::testStartModule() {
    TestableWatchServiceClient client("client1");
    client.startModule("mod", "params");
}

void TestWatchServiceClient::testCloseModule() {
    TestableWatchServiceClient client("client1");
    client.closeModule("mod");
}

void TestWatchServiceClient::testCloseModules() {
    TestableWatchServiceClient client("client1");
    client.closeModules();
}

void TestWatchServiceClient::testShowSplashScreen() {
    TestableWatchServiceClient client("client1");
    client.showSplashScreen();
}

void TestWatchServiceClient::testHideSplashScreen() {
    TestableWatchServiceClient client("client1");
    client.hideSplashScreen();
}

void TestWatchServiceClient::testSetState() {
    TestableWatchServiceClient client("client1");
    client.setState(1, 2);
}

void TestWatchServiceClient::testResetState() {
    TestableWatchServiceClient client("client1");
    client.resetState();
}

void TestWatchServiceClient::testSubscribeOnCommandReceived() {
    TestableWatchServiceClient client("client1");
    CommandReceivedReceiver obj;
    QVERIFY(client.subscribeOnCommandReceived(&obj));
}

void TestWatchServiceClient::testSubscribeOnCloseCommandReceived() {
    TestableWatchServiceClient client("client1");
    CloseCommandReceivedReceiver obj;
    QVERIFY(client.subscribeOnCloseCommandReceived(&obj));
}

void TestWatchServiceClient::testSubscribeOnDisconnected() {
    TestableWatchServiceClient client("client1");
    DisconnectedReceiver obj;
    QVERIFY(client.subscribeOnDisconnected(&obj));
}

void TestWatchServiceClient::testSubscribeOnModuleClosed() {
    TestableWatchServiceClient client("client1");
    ModuleClosedReceiver obj;
    QVERIFY(client.subscribeOnModuleClosed(&obj));
}

QTEST_MAIN(TestWatchServiceClient)
#include "TestWatchServiceClient.moc"
