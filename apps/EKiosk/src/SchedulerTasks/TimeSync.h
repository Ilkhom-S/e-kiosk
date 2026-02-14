/* @file Реализация задачи синхронизации системного времени терминала. */

#pragma once

// Qt
#include <QtCore/QDateTime>
#include <QtCore/QMutex>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtCore/QWaitCondition>
#include <QtNetwork/QHostAddress>

#include <Common/QtHeadersBegin.h>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ITask.h>

// Модули
#include <Common/ILogable.h>

class NetworkTaskManager;
class NtpClient;
class NtpReply;

//---------------------------------------------------------------------------
class TimeSync : public QObject, public SDK::PaymentProcessor::ITask, private ILogable {
    Q_OBJECT

    NetworkTaskManager *m_Network; /// Менеджер сетевых задач для HTTP запросов

    NtpClient *m_Client;        /// NTP клиент для точной синхронизации времени
    QSet<QUrl> m_TimeSyncHosts; /// Список NTP/HTTP серверов для синхронизации
    bool m_TimeReceived;        /// Флаг успешной синхронизации времени
    bool m_Canceled;            /// Флаг отмены операции

public:
    /// Конструктор задачи синхронизации времени
    TimeSync(const QString &aName, const QString &aLogName, const QString &aParams);

    /// Выполнить синхронизацию системного времени через NTP/HTTP
    virtual void execute();

    /// Остановить выполнение задачи синхронизации
    virtual bool cancel();

    /// подписаться на сигнал окончания задания
    virtual bool subscribeOnComplete(QObject *aReceiver, const char *aSlot);

private slots:
    /// Обработать ответ от NTP сервера
    void ntpReplyReceived(const QHostAddress &aAddress, quint16 aPort, const NtpReply &aReply);
    /// Обработать HTTP ответ от сервера времени
    void httpReplyReceived();

private:
    /// Проверить следующий URL из списка серверов
    void checkNextUrl();

    /// Обработать полученное от сервера время
    void timeReceived(const QDateTime &aServerDateTime);
    /// Обработать смещение локального времени относительно серверного
    void timeOffsetReceived(qint64 aLocalTimeOffset);

    /// Выполнить синхронизацию через NTP протокол
    void ntpCheckMethod(const QUrl &aHost);

    /// Выполнить приблизительную синхронизацию через HTTP Date header
    void httpCheckMethod(const QUrl &aHost);

private slots:
    /// Обработать таймаут запроса к NTP серверу
    void ntpRequestTimeout();

signals:
    void finished(const QString &aName, bool aComplete);
};
