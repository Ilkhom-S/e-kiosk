#include "MessageQueueClient.h"

#include "MessageQueue/MessageQueueConstants.h"

MessageQueueClient::MessageQueueClient() : ILogable(CIMessageQueueClient::DefaultLog) {
    QObject::connect(&m_Socket, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QObject::connect(&m_Socket,
                     SIGNAL(errorOccurred(QAbstractSocket::SocketError)),
                     this,
                     SLOT(onSocketError(QAbstractSocket::SocketError)));
#else
    QObject::connect(&m_Socket,
                     SIGNAL(error(QAbstractSocket::SocketError)),
                     this,
                     SLOT(onSocketError(QAbstractSocket::SocketError)));
#endif
    QObject::connect(&m_Socket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
    // QObject::connect(&m_AnswerTimer, SIGNAL(timeout()), this, SLOT(onSocketDisconnected()));
}

//----------------------------------------------------------------------------
MessageQueueClient::~MessageQueueClient() = default;

//----------------------------------------------------------------------------
bool MessageQueueClient::connect(const QString &aQueueName) {
    m_Socket.connectToHost("127.0.0.1", aQueueName.toUShort());
    m_Socket.waitForConnected(CIMessageQueueClient::ConnectionTimeout);

    return isConnected();
}

//----------------------------------------------------------------------------
void MessageQueueClient::disconnect() {
    m_Socket.disconnectFromHost();
}

//----------------------------------------------------------------------------
bool MessageQueueClient::isConnected() const {
    return (m_Socket.state() == QAbstractSocket::ConnectedState);
}

//----------------------------------------------------------------------------
void MessageQueueClient::sendMessage(const QByteArray &aMessage) {
    if (m_Socket.state() == QTcpSocket::ConnectedState) {
        m_Socket.write(aMessage + '\0');
        m_Socket.flush();
    }
}

//----------------------------------------------------------------------------
bool MessageQueueClient::subscribeOnMessageReceived(QObject *aObject) {
    return QObject::connect(this,
                            SIGNAL(onMessageReceived(QByteArray)),
                            aObject,
                            SLOT(onMessageReceived(QByteArray))) != nullptr;
}

//----------------------------------------------------------------------------
bool MessageQueueClient::subscribeOnDisconnected(QObject *aObject) {
    return QObject::connect(this, SIGNAL(onDisconnected()), aObject, SLOT(onDisconnected())) !=
           nullptr;
}

//----------------------------------------------------------------------------
bool MessageQueueClient::subscribeOnEvents(QObject *aObject) {
    return (QObject::connect(this,
                             SIGNAL(onMessageReceived(QByteArray)),
                             aObject,
                             SLOT(onMessageReceived(QByteArray))) != nullptr) &&
           (QObject::connect(this,
                             SIGNAL(onError(CIMessageQueueClient::ErrorCode, const QString &)),
                             aObject,
                             SLOT(onError(CIMessageQueueClient::ErrorCode, const QString &))) !=
            nullptr) &&
           (QObject::connect(this, SIGNAL(onDisconnected()), aObject, SLOT(onDisconnected())) !=
            nullptr);
}

//----------------------------------------------------------------------------
void MessageQueueClient::onSocketReadyRead() {
    while (m_Socket.bytesAvailable() > 0) {
        QByteArray buffer = m_Socket.readAll();

        toLog(LogLevel::Normal, QString::fromUtf8(buffer));

        m_Buffer += buffer;
        parseInputBuffer(m_Buffer);
    }
}

//----------------------------------------------------------------------------
void MessageQueueClient::onSocketError(QAbstractSocket::SocketError aErrorCode) {
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
    m_AnswerTimer.start(MessageQueueConstants::AnswerFrom_ServerTime);
}

//----------------------------------------------------------------------------
