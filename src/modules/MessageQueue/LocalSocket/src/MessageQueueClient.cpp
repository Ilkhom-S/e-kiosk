#include "MessageQueueClient.h"

#include "MessageQueue/MessageQueueConstants.h"

MessageQueueClient::MessageQueueClient() {
    QObject::connect(&m_Socket, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));
    QObject::connect(&m_Socket,
                     SIGNAL(error(QLocalSocket::LocalSocketError)),
                     this,
                     SLOT(onSocketError(QLocalSocket::LocalSocketError)));
    QObject::connect(&m_Socket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
    // QObject::connect(&m_AnswerTimer, SIGNAL(timeout()), this, SLOT(onSocketDisconnected()));
}

//----------------------------------------------------------------------------
MessageQueueClient::~MessageQueueClient() {}

//----------------------------------------------------------------------------
bool MessageQueueClient::connect(const QString &aQueueName) {
    m_Socket.connectToServer(aQueueName);
    m_Socket.waitForConnected(CIMessageQueueClient::ConnectionTimeout);

    return m_Socket.isOpen();
}

//----------------------------------------------------------------------------
void MessageQueueClient::disconnect() {
    m_Socket.disconnectFromServer();
}

//----------------------------------------------------------------------------
bool MessageQueueClient::isConnected() const {
    return m_Socket.isOpen();
}

//----------------------------------------------------------------------------
void MessageQueueClient::sendMessage(const QByteArray &aMessage) {
    if (m_Socket.state() == QLocalSocket::ConnectedState) {
        m_Socket.write(aMessage + '\0');
        m_Socket.flush();
        m_Socket.waitForBytesWritten(100);
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
    QByteArray buffer = m_Socket.readAll();
    /*if (buffer.indexOf(MessageQueueConstants::PingMessage) >= 0)
    {
            m_AnswerTimer.stop();
            QTimer::singleShot(MessageQueueConstants::PingTime, this, SLOT(pingServer()));
            return;
    }*/
    m_Buffer += buffer;
    parseInputBuffer(m_Buffer);
}

//----------------------------------------------------------------------------
void MessageQueueClient::onSocketError(QLocalSocket::LocalSocketError aErrorCode) {
    emit onError(static_cast<CIMessageQueueClient::ErrorCode>(aErrorCode), m_Socket.errorString());
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
    m_AnswerTimer.start(MessageQueueConstants::AnswerFromServerTime);
}

//----------------------------------------------------------------------------
