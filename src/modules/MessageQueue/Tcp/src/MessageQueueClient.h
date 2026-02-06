#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtNetwork/QTcpSocket>

#include <Common/ILogable.h>

#include "MessageQueue/IMessageQueueClient.h"

class MessageQueueClient : public QObject, public IMessageQueueClient, public ILogable {
    Q_OBJECT

public:
    MessageQueueClient();
    virtual ~MessageQueueClient();

    /// Подключиться к очереди сообщений aQueueName.
    virtual bool connect(const QString &aQueueName) override;
    /// Отключиться от текущей очереди сообщений.
    virtual void disconnect() override;
    /// Возвращает статус подключения.
    virtual bool isConnected() const override;

    /// Послать сообщение серверу.
    virtual void sendMessage(const QByteArray &aMessage) override;

    /// Подписаться на получение сообщения. aObject должен иметь
    /// слот onMessageReceived(QByteArray aMessage).
    virtual bool subscribeOnMessageReceived(QObject *aObject) override;

    /// Подписаться на получение сообщения. aObject должен иметь
    /// слот onDisconnected().
    virtual bool subscribeOnDisconnected(QObject *aObject) override;

    /// Подписывает aObject на все сообщения. aObject должен иметь
    /// слоты onMessageReceived(QByteArray aMessage), onDisconnected(),
    /// onError(CIMessageQueueClient::ErrorCode aErrorCode, const QString & aErrorMessage).
    virtual bool subscribeOnEvents(QObject *aObject) override;

private:
    void parseInputBuffer(QByteArray &aBuffer);

private slots:
    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError aErrorCode);
    void onSocketDisconnected();
    void pingServer();

signals:
    void onMessageReceived(QByteArray aMessage);
    void onError(CIMessageQueueClient::ErrorCode aErrorCode, const QString &aErrorMessage);
    void onDisconnected();

private:
    QTcpSocket mSocket;
    QByteArray mBuffer;

    /// Таймер, который будет следить за ответом сервера на пинг.
    QTimer mAnswerTimer;
};

//----------------------------------------------------------------------------
