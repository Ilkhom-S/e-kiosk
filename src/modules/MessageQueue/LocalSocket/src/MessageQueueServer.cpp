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
    if (!connect(&mDisconnectSignalMapper,
                 SIGNAL(mapped(QObject *)),
                 this,
                 SLOT(onSocketDisconnected(QObject *))) ||
        !connect(&mReadyReadSignalMapper,
                 SIGNAL(mapped(QObject *)),
                 this,
                 SLOT(onSocketReadyRead(QObject *))))
        return false;

    if (!isListening())
        return listen(mQueueName);

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
    foreach (QLocalSocket *socket, mSockets.keys()) {
        if (socket->state() == QLocalSocket::ConnectedState) {
            socket->write(aMessage + '\0');
            socket->flush();
            socket->waitForBytesWritten(100);
        }
    }
}

//----------------------------------------------------------------------------
void MessageQueueServer::incomingConnection(quintptr aSocketDescriptor) {
    LOG(mLog,
        LogLevel::Normal,
        QString("New incoming connection... Socket with descriptor %1 has been connected.")
            .arg(aSocketDescriptor));

    QLocalSocket *newSocket = new QLocalSocket(this);
    newSocket->setSocketDescriptor(aSocketDescriptor);

    mDisconnectSignalMapper.setMapping(newSocket, newSocket);
    connect(newSocket, SIGNAL(disconnected()), &mDisconnectSignalMapper, SLOT(map()));

    mReadyReadSignalMapper.setMapping(newSocket, newSocket);
    connect(newSocket, SIGNAL(readyRead()), &mReadyReadSignalMapper, SLOT(map()));

    mSockets[newSocket] = aSocketDescriptor;
}

//----------------------------------------------------------------------------
void MessageQueueServer::onSocketDisconnected(QObject *aObject) {
    emit onDisconnected();

    QLocalSocket *socket = dynamic_cast<QLocalSocket *>(aObject);

    if (socket) {
        LOG(mLog,
            LogLevel::Normal,
            QString("Socket with descriptor %1 has been disconnected.").arg(mSockets[socket]));

        mBuffers.remove(mSockets[socket]);
        mSockets.remove(socket);
    }

    aObject->deleteLater();
}

//----------------------------------------------------------------------------
void MessageQueueServer::onSocketReadyRead(QObject *aObject) {
    QLocalSocket *socket = dynamic_cast<QLocalSocket *>(aObject);
    if (!socket) {
        LOG(mLog, LogLevel::Error, "Wrong object was passed to onSocketReadyRead slot...");
        return;
    }

    QByteArray newData = socket->readAll();

    /*if (newData.indexOf(MessageQueueConstants::PingMessage) >= 0)
    {
            sendMessage(MessageQueueConstants::PingMessage);
            return;
    }*/

    quintptr socketDescriptor = socket->socketDescriptor();

    if (mBuffers.contains(socketDescriptor)) {
        mBuffers[socketDescriptor] = mBuffers[socketDescriptor] + newData;
    } else {
        mBuffers[socketDescriptor] = newData;
    }

    parseInputBuffer(mBuffers[socketDescriptor]);
}

//----------------------------------------------------------------------------
void MessageQueueServer::parseInputBuffer(QByteArray &aBuffer) {
    int messageEnd = aBuffer.indexOf('\0');
    while (messageEnd != -1) {
        QByteArray newMessageData = aBuffer.left(messageEnd);

        aBuffer = aBuffer.right(aBuffer.size() - messageEnd - 1);

        emit onMessageReceived(newMessageData);

        messageEnd = aBuffer.indexOf('\0');
    }
}

//----------------------------------------------------------------------------
