#include "MessageQueueClient.h"

#include "MessageQueue/MessageQueueConstants.h"

MessageQueueClient::MessageQueueClient() {
    QObject::connect(&mSocket, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));
    QObject::connect(&mSocket,
                     SIGNAL(error(QLocalSocket::LocalSocketError)),
                     this,
                     SLOT(onSocketError(QLocalSocket::LocalSocketError)));
    QObject::connect(&mSocket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
    // QObject::connect(&mAnswerTimer, SIGNAL(timeout()), this, SLOT(onSocketDisconnected()));
}

//----------------------------------------------------------------------------
MessageQueueClient::~MessageQueueClient() {}

//----------------------------------------------------------------------------
bool MessageQueueClient::connect(const QString &aQueueName) {
    mSocket.connectToServer(aQueueName);
    mSocket.waitForConnected(CIMessageQueueClient::ConnectionTimeout);

    return mSocket.isOpen();
}

//----------------------------------------------------------------------------
void MessageQueueClient::disconnect() {
    mSocket.disconnectFromServer();
}

//----------------------------------------------------------------------------
bool MessageQueueClient::isConnected() const {
    return mSocket.isOpen();
}

//----------------------------------------------------------------------------
void MessageQueueClient::sendMessage(const QByteArray &aMessage) {
    if (mSocket.state() == QLocalSocket::ConnectedState) {
        mSocket.write(aMessage + '\0');
        mSocket.flush();
        mSocket.waitForBytesWritten(100);
    }
}

//----------------------------------------------------------------------------
bool MessageQueueClient::subscribeOnMessageReceived(QObject *aObject) {
    return QObject::connect(
        this, SIGNAL(onMessageReceived(QByteArray)), aObject, SLOT(onMessageReceived(QByteArray)));
}

//----------------------------------------------------------------------------
bool MessageQueueClient::subscribeOnDisconnected(QObject *aObject) {
    return QObject::connect(this, SIGNAL(onDisconnected()), aObject, SLOT(onDisconnected()));
}

//----------------------------------------------------------------------------
bool MessageQueueClient::subscribeOnEvents(QObject *aObject) {
    // QTimer::singleShot(MessageQueueConstants::PingTime, this, SLOT(pingServer()));
    return QObject::connect(this,
                            SIGNAL(onMessageReceived(QByteArray)),
                            aObject,
                            SLOT(onMessageReceived(QByteArray))) &&
           QObject::connect(this,
                            SIGNAL(onError(CIMessageQueueClient::ErrorCode, const QString &)),
                            aObject,
                            SLOT(onError(CIMessageQueueClient::ErrorCode, const QString &))) &&
           QObject::connect(this, SIGNAL(onDisconnected()), aObject, SLOT(onDisconnected()));
}

//----------------------------------------------------------------------------
void MessageQueueClient::onSocketReadyRead() {
    QByteArray buffer = m_socket.readAll();
    /*if (buffer.indexOf(MessageQueueConstants::PingMessage) >= 0)
    {
            mAnswerTimer.stop();
            QTimer::singleShot(MessageQueueConstants::PingTime, this, SLOT(pingServer()));
            return;
    }*/
    mBuffer += buffer;
    parseInputBuffer(mBuffer);
}

//----------------------------------------------------------------------------
void MessageQueueClient::onSocketError(QLocalSocket::LocalSocketError aErrorCode) {
    emit onError(static_cast<CIMessageQueueClient::ErrorCode>(aErrorCode), mSocket.errorString());
}

//----------------------------------------------------------------------------
void MessageQueueClient::onSocketDisconnected() {
    emit onDisconnected();
}

//----------------------------------------------------------------------------
void MessageQueueClient::parseInputBuffer(QByteArray &aBuffer) {
    int messageEnd = aBuffer.indexOf('\0');
    while (messageEnd != -1) {
        QByteArray newMessageData = aBuffer.left(messageEnd);

        aBuffer = aBuffer.right(aBuffer.size() - messageEnd - 1);

        emit onMessageReceived(newMessageData);

        messageEnd = aBuffer.indexOf('\0');
    }
}

//----------------------------------------------------------------------------
void MessageQueueClient::pingServer() {
    sendMessage(MessageQueueConstants::PingMessage);
    mAnswerTimer.start(MessageQueueConstants::AnswerFromServerTime);
}

//----------------------------------------------------------------------------
