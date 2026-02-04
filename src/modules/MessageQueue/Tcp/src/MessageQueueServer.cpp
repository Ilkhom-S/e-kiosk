// System
#include "MessageQueue/MessageQueueConstants.h"

// Project
#include "MessageQueueServer.h"

MessageQueueServer::MessageQueueServer(const QString &aQueueName)
{
    m_log = ILog::getInstance(CIMessageQueueServer::DefaultLog);

    m_queueName = aQueueName;
}

//----------------------------------------------------------------------------
MessageQueueServer::MessageQueueServer(const QString &aQueueName, ILog *aLog)
{
    m_log = aLog;

    m_queueName = aQueueName;
}

//----------------------------------------------------------------------------
MessageQueueServer::~MessageQueueServer()
{
}

//----------------------------------------------------------------------------
bool MessageQueueServer::init()
{
    // No initialization needed for signal mappers in Qt6 - using lambda connections
    if (!isListening())
    {
        bool result = listen(QHostAddress::LocalHost, static_cast<qint16>(m_queueName.toInt()));
        if (!result)
        {
            LOG(m_log, LogLevel::Error, QString("Failed to listen on port %1: %2").arg(m_queueName).arg(errorString()));
        }
        return result;
    }

    return true;
}

//----------------------------------------------------------------------------
void MessageQueueServer::stop()
{
    QTcpServer::close();
}

//----------------------------------------------------------------------------
bool MessageQueueServer::subscribeOnMessageReceived(QObject *aObject)
{
    return connect(this, SIGNAL(onMessageReceived(QByteArray)), aObject, SLOT(onMessageReceived(QByteArray)));
}

//----------------------------------------------------------------------------
bool MessageQueueServer::subscribeOnDisconnected(QObject *aObject)
{
    return connect(this, SIGNAL(onDisconnected()), aObject, SLOT(onDisconnected()));
}

//----------------------------------------------------------------------------
void MessageQueueServer::sendMessage(const QByteArray &aMessage)
{
    foreach (QTcpSocket *socket, m_sockets.keys())
    {
        if (socket->state() == QTcpSocket::ConnectedState)
        {
            socket->write(aMessage + '\0');
            socket->flush();
        }
    }
}

//----------------------------------------------------------------------------
void MessageQueueServer::incomingConnection(int aSocketDescriptor)
{
    LOG(m_log, LogLevel::Normal,
        QString("New incoming connection... Socket with descriptor %1 has been connected.").arg(aSocketDescriptor));

    QTcpSocket *newSocket = new QTcpSocket(this);
    newSocket->setSocketDescriptor(aSocketDescriptor);

    // Use lambda connections instead of QSignalMapper for Qt6 compatibility
    connect(newSocket, &QTcpSocket::disconnected, this, [this, newSocket]() { onSocketDisconnected(newSocket); });

    connect(newSocket, &QTcpSocket::readyRead, this, [this, newSocket]() { onSocketReadyRead(newSocket); });

    m_sockets[newSocket] = aSocketDescriptor;
}

//----------------------------------------------------------------------------
void MessageQueueServer::onSocketDisconnected()
{
    // This method is no longer used with lambda connections
    // Kept for compatibility but should not be called
}

//----------------------------------------------------------------------------
void MessageQueueServer::onSocketReadyRead()
{
    // This method is no longer used with lambda connections
    // Kept for compatibility but should not be called
}

//----------------------------------------------------------------------------
void MessageQueueServer::onSocketDisconnected(QTcpSocket *socket)
{
    emit onDisconnected();

    if (socket)
    {
        LOG(m_log, LogLevel::Normal,
            QString("Socket with descriptor %1 has been disconnected.").arg(m_sockets[socket]));

        m_buffers.remove(m_sockets[socket]);
        m_sockets.remove(socket);
    }

    socket->deleteLater();
}

//----------------------------------------------------------------------------
void MessageQueueServer::onSocketReadyRead(QTcpSocket *socket)
{
    if (!socket)
    {
        LOG(m_log, LogLevel::Error, "Wrong socket passed to onSocketReadyRead slot...");
        return;
    }

    while (socket->bytesAvailable() > 0)
    {
        QByteArray newData = socket->readAll();

        quintptr socketDescriptor = socket->socketDescriptor();

        m_buffers[socketDescriptor] = parseInputBuffer(m_buffers[socketDescriptor].append(newData));
    }
}

//----------------------------------------------------------------------------
QByteArray MessageQueueServer::parseInputBuffer(QByteArray aBuffer)
{
    int messageEnd = aBuffer.indexOf('\0');
    while (messageEnd != -1)
    {
        QByteArray newMessageData = aBuffer.left(messageEnd);

        aBuffer = aBuffer.right(aBuffer.size() - messageEnd - 1);

        emit onMessageReceived(newMessageData);

        messageEnd = aBuffer.indexOf('\0');
    }

    return aBuffer;
}

//----------------------------------------------------------------------------
