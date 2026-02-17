#include "MessageQueueServer.h"

#include <utility>

#include "MessageQueue/MessageQueueConstants.h"

MessageQueueServer::MessageQueueServer(QString aQueueName)
    : m_Log(ILog::getInstance(CIMessageQueueServer::DefaultLog)),
      m_QueueName(std::move(aQueueName)) {}

//----------------------------------------------------------------------------
MessageQueueServer::MessageQueueServer(QString aQueueName, ILog *aLog)
    : m_Log(aLog), m_QueueName(std::move(aQueueName)) {}

//----------------------------------------------------------------------------
MessageQueueServer::~MessageQueueServer() = default;

//----------------------------------------------------------------------------
bool MessageQueueServer::init() {
    // No initialization needed for signal mappers in Qt6 - using lambda connections
    if (!isListening()) {
        bool result = listen(QHostAddress::LocalHost, static_cast<qint16>(m_QueueName.toInt()));
        if (!result) {
            LOG(m_Log,
                LogLevel::Error,
                QString("Failed to listen on port %1: %2").arg(m_QueueName).arg(errorString()));
        }
        return result;
    }

    return true;
}

//----------------------------------------------------------------------------
void MessageQueueServer::stop() {
    QTcpServer::close();
}

//----------------------------------------------------------------------------
bool MessageQueueServer::subscribeOnMessageReceived(QObject *aObject) {
    return connect(this,
                   SIGNAL(onMessageReceived(QByteArray)),
                   aObject,
                   SLOT(onMessageReceived(QByteArray))) != nullptr;
}

//----------------------------------------------------------------------------
bool MessageQueueServer::subscribeOnDisconnected(QObject *aObject) {
    return connect(this, SIGNAL(onDisconnected()), aObject, SLOT(onDisconnected())) != nullptr;
}

//----------------------------------------------------------------------------
void MessageQueueServer::sendMessage(const QByteArray &aMessage) {
    foreach (QTcpSocket *socket, m_Sockets.keys()) {
        if (socket->state() == QTcpSocket::ConnectedState) {
            socket->write(aMessage + '\0');
            socket->flush();
        }
    }
}

//----------------------------------------------------------------------------
void MessageQueueServer::incomingConnection(qintptr aSocketDescriptor) {
    LOG(m_Log,
        LogLevel::Normal,
        QString("New incoming connection... Socket with descriptor %1 has been connected.")
            .arg(aSocketDescriptor));

    auto *newSocket = new QTcpSocket(this);
    // Use the qintptr overload (Qt5/Qt6 compatible) to avoid narrowing/casting
    newSocket->setSocketDescriptor(aSocketDescriptor);

    // Use lambda connections instead of QSignalMapper for Qt6 compatibility
    connect(newSocket, &QTcpSocket::disconnected, this, [this, newSocket]() {
        onSocketDisconnected(newSocket);
    });

    connect(newSocket, &QTcpSocket::readyRead, this, [this, newSocket]() {
        onSocketReadyRead(newSocket);
    });

    m_Sockets[newSocket] = static_cast<quintptr>(aSocketDescriptor);
}

//----------------------------------------------------------------------------
void MessageQueueServer::onSocketDisconnected() {
    // This method is no longer used with lambda connections
    // Kept for compatibility but should not be called
}

//----------------------------------------------------------------------------
void MessageQueueServer::onSocketReadyRead() {
    // This method is no longer used with lambda connections
    // Kept for compatibility but should not be called
}

//----------------------------------------------------------------------------
void MessageQueueServer::onSocketDisconnected(QTcpSocket *socket) {
    emit onDisconnected();

    if (socket) {
        LOG(m_Log,
            LogLevel::Normal,
            QString("Socket with descriptor %1 has been disconnected.").arg(m_Sockets[socket]));

        m_Buffers.remove(m_Sockets[socket]);
        m_Sockets.remove(socket);
    }

    socket->deleteLater();
}

//----------------------------------------------------------------------------
void MessageQueueServer::onSocketReadyRead(QTcpSocket *socket) {
    if (!socket) {
        LOG(m_Log, LogLevel::Error, "Wrong socket passed to onSocketReadyRead slot...");
        return;
    }

    while (socket->bytesAvailable() > 0) {
        QByteArray newData = socket->readAll();

        quintptr socketDescriptor = socket->socketDescriptor();

        m_Buffers[socketDescriptor] = parseInputBuffer(m_Buffers[socketDescriptor].append(newData));
    }
}

//----------------------------------------------------------------------------
QByteArray MessageQueueServer::parseInputBuffer(QByteArray aBuffer) {
    int messageEnd = aBuffer.indexOf('\0');
    while (messageEnd != -1) {
        QByteArray newMessageData = aBuffer.left(messageEnd);

        aBuffer = aBuffer.right(aBuffer.size() - messageEnd - 1);

        emit onMessageReceived(newMessageData);

        messageEnd = aBuffer.indexOf('\0');
    }

    return aBuffer;
}

//----------------------------------------------------------------------------
