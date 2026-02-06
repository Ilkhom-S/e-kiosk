#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QSignalMapper>
#include <QtCore/QString>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Common/ILog.h"

// System
#include "MessageQueue/IMessageQueueServer.h"

class MessageQueueServer : public QLocalServer, public IMessageQueueServer
{
    typedef QMap<QLocalSocket *, quintptr> TLocalSocketMap;
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
    virtual void incomingConnection(quintptr socketDescriptor) override;

  private:
    void parseInputBuffer(QByteArray &aBuffer);

  signals:
    void onMessageReceived(QByteArray aMessage);
    void onDisconnected();

  private slots:
    void onSocketDisconnected(QObject *aObject);
    void onSocketReadyRead(QObject *aObject);

  private:
    QSignalMapper mDisconnectSignalMapper;
    QSignalMapper mReadyReadSignalMapper;
    TLocalSocketMap mSockets;
    TSocketBufferMap mBuffers;
    QString mQueueName;
    ILog *mLog;
};

//----------------------------------------------------------------------------
