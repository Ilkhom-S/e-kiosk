// System
#include "MessageQueue/MessageQueueConstants.h"

// Project
#include "MessageQueueClient.h"

MessageQueueClient::MessageQueueClient() : ILogable(CIMessageQueueClient::DefaultLog)
{
    QObject::connect(&mSocket, SIGNAL(readyRead()), this, SLOT(onSocketReadyRead()));
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QObject::connect(&mSocket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this,
                     SLOT(onSocketError(QAbstractSocket::SocketError)));
#else
    QObject::connect(&mSocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
                     SLOT(onSocketError(QAbstractSocket::SocketError)));
#endif
    QObject::connect(&mSocket, SIGNAL(disconnected()), this, SLOT(onSocketDisconnected()));
    // QObject::connect(&mAnswerTimer, SIGNAL(timeout()), this, SLOT(onSocketDisconnected()));
}

//----------------------------------------------------------------------------
MessageQueueClient::~MessageQueueClient()
{
}

//----------------------------------------------------------------------------
bool MessageQueueClient::connect(const QString &aQueueName)
{
    mSocket.connectToHost("127.0.0.1", aQueueName.toUShort());
    mSocket.waitForConnected(CIMessageQueueClient::ConnectionTimeout);

    return isConnected();
}

//----------------------------------------------------------------------------
void MessageQueueClient::disconnect()
{
    mSocket.disconnectFromHost();
}

//----------------------------------------------------------------------------
bool MessageQueueClient::isConnected() const
{
    return (mSocket.state() == QAbstractSocket::ConnectedState);
}

//----------------------------------------------------------------------------
void MessageQueueClient::sendMessage(const QByteArray &aMessage)
{
    if (mSocket.state() == QTcpSocket::ConnectedState)
    {
        mSocket.write(aMessage + '\0');
        mSocket.flush();
    }
}

//----------------------------------------------------------------------------
bool MessageQueueClient::subscribeOnMessageReceived(QObject *aObject)
{
    return QObject::connect(this, SIGNAL(onMessageReceived(QByteArray)), aObject, SLOT(onMessageReceived(QByteArray)));
}

//----------------------------------------------------------------------------
bool MessageQueueClient::subscribeOnDisconnected(QObject *aObject)
{
    return QObject::connect(this, SIGNAL(onDisconnected()), aObject, SLOT(onDisconnected()));
}

//----------------------------------------------------------------------------
bool MessageQueueClient::subscribeOnEvents(QObject *aObject)
{
    return QObject::connect(this, SIGNAL(onMessageReceived(QByteArray)), aObject,
                            SLOT(onMessageReceived(QByteArray))) &&
           QObject::connect(this, SIGNAL(onError(CIMessageQueueClient::ErrorCode, const QString &)), aObject,
                            SLOT(onError(CIMessageQueueClient::ErrorCode, const QString &))) &&
           QObject::connect(this, SIGNAL(onDisconnected()), aObject, SLOT(onDisconnected()));
}

//----------------------------------------------------------------------------
void MessageQueueClient::onSocketReadyRead()
{
    while (mSocket.bytesAvailable() > 0)
    {
        QByteArray buffer = mSocket.readAll();

        toLog(LogLevel::Normal, QString::fromUtf8(buffer));

        mBuffer += buffer;
        parseInputBuffer(mBuffer);
    }
}

//----------------------------------------------------------------------------
void MessageQueueClient::onSocketError(QAbstractSocket::SocketError aErrorCode)
{
    emit onError(static_cast<CIMessageQueueClient::ErrorCode>(aErrorCode), mSocket.errorString());
}

//----------------------------------------------------------------------------
void MessageQueueClient::onSocketDisconnected()
{
    emit onDisconnected();
}

//----------------------------------------------------------------------------
void MessageQueueClient::parseInputBuffer(QByteArray &aBuffer)
{
    int messageEnd = aBuffer.indexOf('\0');
    while (messageEnd != -1)
    {
        QByteArray newMessageData = aBuffer.left(messageEnd);

        aBuffer = aBuffer.right(aBuffer.size() - messageEnd - 1);

        emit onMessageReceived(newMessageData);

        messageEnd = aBuffer.indexOf('\0');
    }
}

//----------------------------------------------------------------------------
void MessageQueueClient::pingServer()
{
    sendMessage(MessageQueueConstants::PingMessage);
    mAnswerTimer.start(MessageQueueConstants::AnswerFromServerTime);
}

//----------------------------------------------------------------------------
