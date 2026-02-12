#include <QDebug>
#include <QSignalSpy>
#include <QtTest/QtTest>

#include <WatchServiceClient/Constants.h>

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

class TestWatchServiceClient : public QObject {
    Q_OBJECT

private slots:
    void testCloseCommandEmitted();
    void testModuleClosedEmitted();
    void testGenericCommandEmitted();
    void testTargetFiltering();
};

void TestWatchServiceClient::testCloseCommandEmitted() {
    TestableWatchServiceClient client("client1");

    QSignalSpy spy(&client, SIGNAL(onCloseCommandReceived()));

    QByteArray msg = QString("%1=%2;%3=%4;")
                         .arg(CWatchService::Fields::Sender)
                         .arg(CWatchService::Name)
                         .arg(CWatchService::Fields::Type)
                         .arg(CWatchService::Commands::Exit)
                         .toUtf8();

    client.callOnMessage(msg);

    QCOMPARE(spy.count(), 1);
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

QTEST_MAIN(TestWatchServiceClient)
#include "TestWatchServiceClient.moc"
