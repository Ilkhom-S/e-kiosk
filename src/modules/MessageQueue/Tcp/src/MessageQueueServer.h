#pragma once

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

#include <Common/ILog.h>

#include "MessageQueue/IMessageQueueServer.h"

class MessageQueueServer : public QTcpServer, public IMessageQueueServer {
    typedef QMap<QTcpSocket *, quintptr> TLocalSocketMap;
    typedef QMap<quintptr, QByteArray> TSocketBufferMap;

    Q_OBJECT

public:
    MessageQueueServer(const QString &aName);
    MessageQueueServer(const QString &aName, ILog *aLog);
    virtual ~MessageQueueServer();

    /// Активировать очередь сообщений.
    virtual bool init() override;
    /// Останавливает сервер.
    virtual void stop() override;

    /// Послать сообщение всем подключенным клиентам.
    virtual void sendMessage(const QByteArray &aMessage) override;

    /// Подписаться на получение сообщения. aObject должен иметь
    /// слот onMessageReceived(QByteArray aMessage).
    virtual bool subscribeOnMessageReceived(QObject *aObject) override;

    /// Подписаться на получение сообщения. aObject должен иметь
    /// слот onDisconnected().
    virtual bool subscribeOnDisconnected(QObject *aObject) override;

protected:
    virtual void incomingConnection(int socketDescriptor);

private:
    QByteArray parseInputBuffer(QByteArray aBuffer);

signals:
    void onMessageReceived(QByteArray aMessage);
    void onDisconnected();

private slots:
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketDisconnected(QTcpSocket *socket);
    void onSocketReadyRead(QTcpSocket *socket);

private:
    TLocalSocketMap m_Sockets;
    TSocketBufferMap m_Buffers;
    QString m_QueueName;
    ILog *m_Log;
};

//----------------------------------------------------------------------------
