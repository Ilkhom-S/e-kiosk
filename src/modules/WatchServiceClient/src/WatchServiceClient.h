/* @file Реализация клиента сторожевого сервиса. */

#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>

#include <MessageQueue/IMessageQueueClient.h>
#include <WatchServiceClient/IWatchServiceClient.h>
#include <boost/function.hpp>

//---------------------------------------------------------------------------
class WatchServiceClient : public QThread, public IWatchServiceClient {
    Q_OBJECT

public:
    WatchServiceClient(QString aName, PingThread aThread);
    virtual ~WatchServiceClient();

    /// IWatchServiceClient: Подключение к сторожевому сервису.
    virtual bool start() override;

    /// IWatchServiceClient: Закрытие соединения со сторожевым сервисом.
    virtual void stop() override;

    /// IWatchServiceClient: Возвращает true, если клиент подключён к сторожевому сервису.
    virtual bool isConnected() const override;

    /// IWatchServiceClient: Выполнение команды aCommand на модуле aModule c параметрами aParams.
    virtual void
    execute(QString aCommand, QString aModule = QString(), QString aParams = QString()) override;

    /// IWatchServiceClient: Завершение работы всех модулей и сторожевого сервиса.
    virtual void stopService() override;

    /// IWatchServiceClient: Команда перезапуска сервиса.
    virtual void restartService(QStringList aParameters) override;

    /// IWatchServiceClient: Перезагрузка компьютера.
    virtual void rebootMachine() override;

    /// IWatchServiceClient: Выключение компьютера.
    virtual void shutdownMachine() override;

    /// IWatchServiceClient: Запуск модуля aModule с параметрами aParams.
    virtual void startModule(QString aModule, QString aParams = QString()) override;

    /// IWatchServiceClient: Закрытие модуля aModule.
    virtual void closeModule(QString aModule) override;

    /// Закрытие всех активных модулей без закрытия сервиса.
    virtual void closeModules() override;

    /// IWatchServiceClient: Включение защитного экрана.
    virtual void showSplashScreen() override;

    /// IWatchServiceClient: Отключение защитного экрана.
    virtual void hideSplashScreen() override;

    /// IWatchServiceClient: Сообщить сервису о статусе aStatus состояния aType.
    virtual void setState(int aType, int aStatus) override;

    /// IWatchServiceClient: Сбрасывает все установленные статусы.
    virtual void resetState() override;

    /// IWatchServiceClient: Подписывает aObject на получение команд от сервиса. Объект должен иметь
    /// слот с сигнатурой onCommandReceived(const QString& aSender, const QString& aTarget,
    /// const QString& aType, const QStringList & aTail).
    virtual bool subscribeOnCommandReceived(QObject *aObject) override;

    /// IWatchServiceClient: Подписывает aObject на получение оповещения от сервиса о требованием
    /// завершить работу. Объект должен иметь слот с сигнатурой onCloseCommandReceived().
    virtual bool subscribeOnCloseCommandReceived(QObject *aObject) override;

    /// IWatchServiceClient: Подписывает aObject на получение оповещения о разрыве связи со
    /// сторожевым сервисом. Объект должен иметь слот с сигнатурой onDisconnected().
    virtual bool subscribeOnDisconnected(QObject *aObject) override;

    /// Подписывает aObject на получение оповещения от сервиса о закрытии любого модуля.
    /// Объект должен иметь слот с сигнатурой onModuleClosed(const QString &).
    virtual bool subscribeOnModuleClosed(QObject *aObject) override;

protected:
    /// Рабочая процедура Qt'шной нитки.
    virtual void run() override;

    /// Пинг сторожевого сервиса.
    virtual void ping();

private:
    typedef boost::function<void()> TMethod;

    // Отправка сообщения по транспортному каналу.
    void sendMessage(const QByteArray &aMessage);

signals:
    /// Вызов указанного метода в своём потоке.
    void invokeMethod(WatchServiceClient::TMethod aMethod);

    /// Клиент отключился от сторожевого сервиса.
    void disconnected();

    /// От сервера получена команда на завершение работы.
    void onCloseCommandReceived();

    /// Сигнал, сообщающий о закрытии модуля
    void onModuleClosed(const QString &aModuleName);

    /// От севера получена команда.
    void onCommandReceived(const QString &aSender,
                           const QString &aTarget,
                           const QString &aType,
                           const QStringList &aTail);

protected slots:
    /// Обработчик при вызове метода из родного потока.
    static void onInvokeMethod(const WatchServiceClient::TMethod &aMethod);

    /// Обработчик для пинга сторожевого сервиса.
    void onPing();

    /// Обработчик получаемых от сервера данных.
    void onMessageReceived(QByteArray aMessage);

    /// Получаем сообщение об отключении с транспортного уровня.
    void onDisconnected();

private:
    QSharedPointer<IMessageQueueClient> m_Client;

    QTimer m_PingTimer;

    QString m_Name;

    QWaitCondition m_InitCondition;
    QMutex m_InitMutex;
};

//---------------------------------------------------------------------------
