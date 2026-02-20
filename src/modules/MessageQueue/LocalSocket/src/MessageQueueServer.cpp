#include "MessageQueueServer.h"

#include "MessageQueue/MessageQueueConstants.h"

MessageQueueServer::MessageQueueServer(const QString &aQueueName)
    : m_Log(ILog::getInstance(CIMessageQueueServer::DefaultLog)) {

    m_QueueName = aQueueName;
}

//----------------------------------------------------------------------------
MessageQueueServer::MessageQueueServer(const QString &aQueueName, ILog *aLog) : m_Log(aLog) {

    m_QueueName = aQueueName;
}

//----------------------------------------------------------------------------
MessageQueueServer::~MessageQueueServer() = default;

//----------------------------------------------------------------------------
bool MessageQueueServer::init() {
    if (!isListening()) {
        return listen(m_QueueName);
    }

    return true;
}

//----------------------------------------------------------------------------
void MessageQueueServer::stop() {
    QLocalServer::close();
}

//----------------------------------------------------------------------------
bool MessageQueueServer::subscribeOnMessageReceived(QObject *aObject) {
    return connect(
        this, SIGNAL(onMessageReceived(QByteArray)), aObject, SLOT(onMessageReceived(QByteArray)));
}

//----------------------------------------------------------------------------
bool MessageQueueServer::subscribeOnDisconnected(QObject *aObject) {
    return connect(this, SIGNAL(onDisconnected()), aObject, SLOT(onDisconnected()));
}

//----------------------------------------------------------------------------
void MessageQueueServer::sendMessage(const QByteArray &aMessage) {
    foreach (QLocalSocket *socket, m_Sockets.keys()) {
        if (socket->state() == QLocalSocket::ConnectedState) {
            socket->write(aMessage + '\0');
            socket->flush();
            socket->waitForBytesWritten(100);
        }
    }
}

//----------------------------------------------------------------------------
void MessageQueueServer::incomingConnection(quintptr aSocketDescriptor) {
    LOG(m_Log,
        LogLevel::Normal,
        QString("New incoming connection... Socket with descriptor %1 has been connected.")
            .arg(aSocketDescriptor));

    QLocalSocket *newSocket = new QLocalSocket(this);
    newSocket->setSocketDescriptor(aSocketDescriptor);

    m_Sockets[newSocket] = aSocketDescriptor;

    // Используем lambda-соединения вместо устаревшего QSignalMapper
    connect(newSocket, &QLocalSocket::disconnected, this, [this, newSocket]() {
        onSocketDisconnected();
    });

    connect(newSocket, &QLocalSocket::readyRead, this, [this, newSocket]() {
        onSocketReadyRead(newSocket);
    });
}

//----------------------------------------------------------------------------
void MessageQueueServer::onSocketDisconnected() {
    emit onDisconnected();

    // Find the socket that was disconnected
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());

    if (socket) {
        LOG(m_Log,
            LogLevel::Normal,
            QString("Socket with descriptor %1 has been disconnected.").arg(m_Sockets[socket]));

        m_Buffers.remove(m_Sockets[socket]);
        m_Sockets.remove(socket);
        socket->deleteLater();
    }
}

//----------------------------------------------------------------------------
void MessageQueueServer::onSocketReadyRead(QObject *aObject) {
    QLocalSocket *socket = dynamic_cast<QLocalSocket *>(aObject);
    if (!socket) {
        LOG(m_Log, LogLevel::Error, "Wrong object was passed to onSocketReadyRead slot...");
        return;
    }

    QByteArray newData = socket->readAll();

    /*if (newData.indexOf(MessageQueueConstants::PingMessage) >= 0)
    {
            sendMessage(MessageQueueConstants::PingMessage);
            return;
    }*/

    quintptr socketDescriptor = socket->socketDescriptor();

    if (m_Buffers.contains(socketDescriptor)) {
        m_Buffers[socketDescriptor] = m_Buffers[socketDescriptor] + newData;
    } else {
        m_Buffers[socketDescriptor] = newData;
    }

    parseInputBuffer(m_Buffers[socketDescriptor]);
}

//----------------------------------------------------------------------------
static void MessageQueueServer::parseInputBuffer(QByteArray &aBuffer) {
    int messageEnd = aBuffer.indexOf('\0');
    while (messageEnd != -1) {
        QByteArray newMessageData = aBuffer.left(messageEnd);

        aBuffer = aBuffer.right(aBuffer.size() - messageEnd - 1);

        emit onMessageReceived(newMessageData);

        messageEnd = aBuffer.indexOf('\0');
    }
}

//----------------------------------------------------------------------------
