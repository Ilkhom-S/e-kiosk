#include "MessageQueueServer.h"

#include "MessageQueue/MessageQueueConstants.h"

MessageQueueServer::MessageQueueServer(const QString &aQueueName) {
    mLog = ILog::getInstance(CIMessageQueueServer::DefaultLog);

    mQueueName = aQueueName;
}

//----------------------------------------------------------------------------
MessageQueueServer::MessageQueueServer(const QString &aQueueName, ILog *aLog) {
    mLog = aLog;

    mQueueName = aQueueName;
}

//----------------------------------------------------------------------------
MessageQueueServer::~MessageQueueServer() {}

//----------------------------------------------------------------------------
bool MessageQueueServer::init() {
    // No initialization needed for signal mappers in Qt6 - using lambda connections
    if (!isListening()) {
        bool result = listen(QHostAddress::LocalHost, static_cast<qint16>(mQueueName.toInt()));
        if (!result) {
            LOG(mLog,
                LogLevel::Error,
                QString("Failed to listen on port %1: %2").arg(mQueueName).arg(errorString()));
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
    return connect(
        this, SIGNAL(onMessageReceived(QByteArray)), aObject, SLOT(onMessageReceived(QByteArray)));
}

//----------------------------------------------------------------------------
bool MessageQueueServer::subscribeOnDisconnected(QObject *aObject) {
    return connect(this, SIGNAL(onDisconnected()), aObject, SLOT(onDisconnected()));
}

//----------------------------------------------------------------------------
void MessageQueueServer::sendMessage(const QByteArray &aMessage) {
    foreach (QTcpSocket *socket, mSockets.keys()) {
        if (socket->state() == QTcpSocket::ConnectedState) {
            socket->write(aMessage + '\0');
            socket->flush();
        }
    }
}

//----------------------------------------------------------------------------
void MessageQueueServer::incomingConnection(int aSocketDescriptor) {
    LOG(mLog,
        LogLevel::Normal,
        QString("New incoming connection... Socket with descriptor %1 has been connected.")
            .arg(aSocketDescriptor));

    QTcpSocket *newSocket = new QTcpSocket(this);
    newSocket->setSocketDescriptor(aSocketDescriptor);

    // Use lambda connections instead of QSignalMapper for Qt6 compatibility
    connect(newSocket, &QTcpSocket::disconnected, this, [this, newSocket]() {
        onSocketDisconnected(newSocket);
    });

    connect(newSocket, &QTcpSocket::readyRead, this, [this, newSocket]() {
        onSocketReadyRead(newSocket);
    });

    mSockets[newSocket] = aSocketDescriptor;
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
        LOG(mLog,
            LogLevel::Normal,
            QString("Socket with descriptor %1 has been disconnected.").arg(mSockets[socket]));

        mBuffers.remove(mSockets[socket]);
        mSockets.remove(socket);
    }

    socket->deleteLater();
}

//----------------------------------------------------------------------------
void MessageQueueServer::onSocketReadyRead(QTcpSocket *socket) {
    if (!socket) {
        LOG(mLog, LogLevel::Error, "Wrong socket passed to onSocketReadyRead slot...");
        return;
    }

    while (socket->bytesAvailable() > 0) {
        QByteArray newData = socket->readAll();

        quintptr socketDescriptor = socket->socketDescriptor();

        mBuffers[socketDescriptor] = parseInputBuffer(mBuffers[socketDescriptor].append(newData));
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
