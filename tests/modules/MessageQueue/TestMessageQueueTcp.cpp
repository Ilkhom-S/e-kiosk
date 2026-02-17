#include <QSignalSpy>
#include <QtTest/QtTest>

#include <MessageQueue/MessageQueueConstants.h>

#include "MessageQueueClient.h"
#include "MessageQueueServer.h"

// Minimal QObject to receive QByteArray signals (used only where QSignalSpy isn't
// convenient).
class MessageReceiver : public QObject {
    Q_OBJECT
public slots:
    void onMessageReceived(const QByteArray &msg) { m_last = msg; }

public:
    QByteArray m_last;
};

class TestMessageQueueTcp : public QObject {
    Q_OBJECT

private slots:
    void testServerStartStop();
    void testClientServerConnectDisconnect();
    void testClientToServerMessage();
    void testServerToClientMessage();
    void testMultipleMessagesInSingleRead();
    void testPartialMessageBoundary();
    void testSendToMultipleClients();
};

void TestMessageQueueTcp::testServerStartStop() {
    MessageQueueServer server("0");

    QVERIFY(server.init());
    QVERIFY(server.isListening());

    // serverPort() must be > 0 when listening on port 0
    QVERIFY(server.serverPort() > 0);

    server.stop();
    QVERIFY(!server.isListening());
}

void TestMessageQueueTcp::testClientServerConnectDisconnect() {
    MessageQueueServer server("0");
    QVERIFY(server.init());

    MessageQueueClient client;

    // connect to server using actual assigned port
    QVERIFY(client.connect(QString::number(server.serverPort())));
    QVERIFY(client.isConnected());

    QSignalSpy serverDisconnectedSpy(&server, SIGNAL(onDisconnected()));
    QSignalSpy clientDisconnectedSpy(&client, SIGNAL(onDisconnected()));

    client.disconnect();

    QTRY_COMPARE(clientDisconnectedSpy.count(), 1);
    // server should also receive a disconnected notification for that socket
    QTRY_COMPARE(serverDisconnectedSpy.count(), 1);
}

void TestMessageQueueTcp::testClientToServerMessage() {
    MessageQueueServer server("0");
    QVERIFY(server.init());

    MessageQueueClient client;
    QVERIFY(client.connect(QString::number(server.serverPort())));

    QSignalSpy serverSpy(&server, SIGNAL(onMessageReceived(QByteArray)));

    client.sendMessage("hello_from_client");

    QTRY_COMPARE(serverSpy.count(), 1);
    QList<QVariant> args = serverSpy.takeFirst();
    QCOMPARE(args.at(0).toByteArray(), QByteArray("hello_from_client"));
}

void TestMessageQueueTcp::testServerToClientMessage() {
    MessageQueueServer server("0");
    QVERIFY(server.init());

    MessageQueueClient client;
    QVERIFY(client.connect(QString::number(server.serverPort())));
    QTRY_VERIFY(client.isConnected());

    QSignalSpy clientSpy(&client, SIGNAL(onMessageReceived(QByteArray)));

    // ensure server has accepted the client's connection (handshake)
    QSignalSpy serverSpy(&server, SIGNAL(onMessageReceived(QByteArray)));
    client.sendMessage("ping");
    QTRY_COMPARE(serverSpy.count(), 1);

    server.sendMessage("server_hello");

    // MessageQueueClient should get an onMessageReceived
    QTRY_COMPARE(clientSpy.count(), 1);
    QList<QVariant> args = clientSpy.takeFirst();
    QCOMPARE(args.at(0).toByteArray(), QByteArray("server_hello"));
}

void TestMessageQueueTcp::testMultipleMessagesInSingleRead() {
    MessageQueueServer server("0");
    QVERIFY(server.init());

    // use raw QTcpSocket to ensure multiple messages arrive in a single read
    QTcpSocket sock;
    sock.connectToHost(QHostAddress::LocalHost, server.serverPort());
    QVERIFY(sock.waitForConnected(5000));

    QSignalSpy serverSpy(&server, SIGNAL(onMessageReceived(QByteArray)));

    QByteArray combined;
    combined.append("msg1");
    combined.append('\0');
    combined.append("msg2");
    combined.append('\0');

    sock.write(combined);
    QVERIFY(sock.waitForBytesWritten(1000));

    QTRY_COMPARE(serverSpy.count(), 2);
    QCOMPARE(serverSpy.takeFirst().at(0).toByteArray(), QByteArray("msg1"));
    QCOMPARE(serverSpy.takeFirst().at(0).toByteArray(), QByteArray("msg2"));

    sock.disconnectFromHost();
}

void TestMessageQueueTcp::testPartialMessageBoundary() {
    MessageQueueServer server("0");
    QVERIFY(server.init());

    QTcpSocket sock;
    sock.connectToHost(QHostAddress::LocalHost, server.serverPort());
    QVERIFY(sock.waitForConnected(5000));

    QSignalSpy serverSpy(&server, SIGNAL(onMessageReceived(QByteArray)));

    // Send partial message (no terminating '\0') and then finish it
    sock.write("partial_");
    QVERIFY(sock.waitForBytesWritten(1000));

    // Ensure server has processed the connection and partial read
    QTRY_VERIFY(sock.state() == QAbstractSocket::ConnectedState);
    QCOMPARE(serverSpy.count(), 0);

    // Finish the message (send terminating '\0')
    sock.write(QByteArray("message") + '\0');
    QVERIFY(sock.waitForBytesWritten(1000));

    QTRY_COMPARE(serverSpy.count(), 1);
    QCOMPARE(serverSpy.takeFirst().at(0).toByteArray(), QByteArray("partial_message"));

    sock.disconnectFromHost();
}

void TestMessageQueueTcp::testSendToMultipleClients() {
    MessageQueueServer server("0");
    QVERIFY(server.init());

    MessageQueueClient c1, c2;
    QVERIFY(c1.connect(QString::number(server.serverPort())));
    QVERIFY(c2.connect(QString::number(server.serverPort())));
    QTRY_VERIFY(c1.isConnected() && c2.isConnected());

    // Ensure the server has accepted both sockets by sending a small handshake
    QSignalSpy serverSpy(&server, SIGNAL(onMessageReceived(QByteArray)));
    c1.sendMessage("ready");
    c2.sendMessage("ready");
    QTRY_COMPARE(serverSpy.count(), 2);

    QSignalSpy spy1(&c1, SIGNAL(onMessageReceived(QByteArray)));
    QSignalSpy spy2(&c2, SIGNAL(onMessageReceived(QByteArray)));

    server.sendMessage("broadcast-msg");

    QTRY_COMPARE(spy1.count(), 1);
    QTRY_COMPARE(spy2.count(), 1);

    QCOMPARE(spy1.takeFirst().at(0).toByteArray(), QByteArray("broadcast-msg"));
    QCOMPARE(spy2.takeFirst().at(0).toByteArray(), QByteArray("broadcast-msg"));
}

QTEST_MAIN(TestMessageQueueTcp)
#include "TestMessageQueueTcp.moc"
