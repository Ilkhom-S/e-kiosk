/* @file Реализация задачи синхронизации системного времени терминала. */

// Qt
#include <QtCore/QCoreApplication>
#include <QtCore/QElapsedTimer>
#include <QtNetwork/QHostInfo>

#include <Common/QtHeadersBegin.h>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Модули
#include <Common/BasicApplication.h>

#include <NetworkTaskManager/MemoryDataStream.h>
#include <NetworkTaskManager/NetworkTask.h>
#include <NetworkTaskManager/NetworkTaskManager.h>
#include <SysUtils/ISysUtils.h>
#include <System/IApplication.h>

// Thirdparty
#include "qntp/NtpClient.h"
#include "qntp/NtpReply.h"

// Проект
#include "Services/TerminalService.h"
#include "TimeSync.h"

namespace PPSDK = SDK::PaymentProcessor;

namespace CTimeSync {
const int DesyncLimit = 60; /// Максимальный предел рассинхронизации времени с сервером в секундах.
const int NtpPort = 123;
const int NtpTimeout = 10;  /// Время в секундах, сколько мы ждем ответа NTP сервера
const int HttpTimeout = 10; /// Таймаут запроса времени по http.

const QString TimeFormat = "yyyy.MM.dd hh:mm:ss";
} // namespace CTimeSync

//---------------------------------------------------------------------------
TimeSync::TimeSync(const QString &aName, const QString &aLogName, const QString &aParams)
    : ITask(aName, aLogName, aParams), ILogable(aLogName), m_Network(nullptr),
      m_Client(new NtpClient(this)), m_TimeReceived(false) {

    connect(m_Client,
            SIGNAL(replyReceived(const QHostAddress &, quint16, const NtpReply &)),
            this,
            SLOT(ntpReplyReceived(const QHostAddress &, quint16, const NtpReply &)),
            Qt::DirectConnection);

    foreach (auto param, aParams.split(",", Qt::SkipEmptyParts)) {
        m_TimeSyncHosts << QUrl(QString("ntp://%1").arg(param.trimmed()));
    }

    auto *app = dynamic_cast<IApplication *>(BasicApplication::getInstance());

    PPSDK::ICore *core = app->getCore();
    PPSDK::TerminalSettings *terminalSettings = dynamic_cast<PPSDK::TerminalSettings *>(
        core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));

    if (core->getNetworkService()) {
        m_Network = core->getNetworkService()->getNetworkTaskManager();
    }

    if (terminalSettings->isValid()) {
        foreach (auto host, terminalSettings->getCheckHosts()) {
            m_TimeSyncHosts << host.first;
        }
    }
}

//---------------------------------------------------------------------------
void TimeSync::checkNextUrl() {
    // Если время уже получено или операция отменена, завершаем задачу
    if (m_TimeReceived || m_Canceled) {
        emit finished(m_Name, m_TimeReceived);

        return;
    }
}

//---------------------------------------------------------------------------
void TimeSync::execute() {
    if (m_TimeSyncHosts.isEmpty()) {
        emit finished(m_Name, false);

        return;
    }

    // Запускаем запросы ко всем серверам сразу (параллельные запросы)
    QSet<QUrl> ntpUrls;

    foreach (auto url, m_TimeSyncHosts) {
        if (url.scheme() == "ntp") {
            ntpUrls << url;

            ntpCheckMethod(url);
        } else {
            httpCheckMethod(url);
        }
    }

    m_TimeSyncHosts.subtract(ntpUrls);

    if (!ntpUrls.isEmpty()) {
        QTimer::singleShot(CTimeSync::NtpTimeout * 1000, this, SLOT(ntpRequestTimeout()));
    }
}

//---------------------------------------------------------------------------
bool TimeSync::cancel() {
    emit finished(m_Name, m_TimeReceived);

    return (m_Canceled = true);
}

//---------------------------------------------------------------------------
bool TimeSync::subscribeOnComplete(QObject *aReceiver, const char *aSlot) {
    return connect(this, SIGNAL(finished(const QString &, bool)), aReceiver, aSlot) != nullptr;
}

//---------------------------------------------------------------------------
void TimeSync::ntpCheckMethod(const QUrl &aHost) {
    toLog(LogLevel::Normal, QString("Get time from %1...").arg(aHost.toString()));

    QHostInfo hostInfo = QHostInfo::fromName(aHost.host());

    if (hostInfo.addresses().isEmpty()) {
        toLog(LogLevel::Error,
              QString("Error IP lookup for '%1'. Skip this server.").arg(aHost.host()));
    } else {
        if (!m_Client->sendRequest(hostInfo.addresses().first(), CTimeSync::NtpPort)) {
            toLog(LogLevel::Error, QString("Failed send NTP sequest to '%1'.").arg(aHost.host()));
        }
    }
}

//---------------------------------------------------------------------------
void TimeSync::ntpRequestTimeout() {
    // Если все сервера опрошены или время уже получено, завершаем
    if (m_TimeSyncHosts.isEmpty() || m_TimeReceived) {
        emit finished(m_Name, m_TimeReceived);

        return;
    }
}

//---------------------------------------------------------------------------
void TimeSync::ntpReplyReceived(const QHostAddress &aAddress,
                                quint16 aPort,
                                const NtpReply &aReply) {
    Q_UNUSED(aAddress)
    Q_UNUSED(aPort)

    // Обрабатываем ответ от NTP сервера
    if (!aReply.isNull()) {
        toLog(LogLevel::Normal,
              QString("Receive NTP reply: server time: %1, local clock offset: %2 ms")
                  .arg(aReply.transmitTime().toLocalTime().toString(CTimeSync::TimeFormat))
                  .arg(aReply.localClockOffset()));

        timeOffsetReceived(aReply.localClockOffset());
    }

    checkNextUrl();
}

//---------------------------------------------------------------------------
void TimeSync::timeReceived(const QDateTime &aServerDateTime) {
    // Проверяем валидность полученного времени и вычисляем смещение
    if (aServerDateTime.isValid()) {
        qint64 clockOffset = QDateTime::currentDateTime().msecsTo(aServerDateTime);

        toLog(LogLevel::Normal,
              QString("Receive HTTP reply: server time: %1, local clock offset: %2 ms")
                  .arg(aServerDateTime.toLocalTime().toString(CTimeSync::TimeFormat))
                  .arg(clockOffset));

        timeOffsetReceived(clockOffset);
    }

    checkNextUrl();
}

//--------------------------------------------------------------------------------
void TimeSync::timeOffsetReceived(qint64 aLocalTimeOffset) {
    if (!m_TimeReceived) {
        m_TimeReceived = true;

        // Если отклонение превышает лимит, корректируем системное время
        if (qAbs(aLocalTimeOffset / 1000) > CTimeSync::DesyncLimit) {
            toLog(LogLevel::Normal,
                  QString("Required time synchronization. Offset %1 sec.")
                      .arg(aLocalTimeOffset / 1000., 0, 'f', 3));

            try {
                ISysUtils::setSystem_Time(QDateTime::currentDateTime().addMSecs(aLocalTimeOffset));

                // После изменения системного времени выполняем ротацию логов
                ILog::logRotateAll();
            } catch (Exception &e) {
                toLog(LogLevel::Error,
                      QString("Failed to set new system date. %1").arg(e.getMessage()));

                m_TimeReceived = false;
            }
        } else {
            toLog(LogLevel::Normal,
                  QString("Time synchronization is not required.  Offset %1 sec.")
                      .arg(aLocalTimeOffset / 1000., 0, 'f', 3));
        }
    }
}

//--------------------------------------------------------------------------------
void TimeSync::httpCheckMethod(const QUrl &aHost) {
    toLog(LogLevel::Normal, QString("Get time from %1...").arg(aHost.toString()));

    if (!m_Network) {
        toLog(LogLevel::Error, "Failed to check connection. Network interface is not specified.");

        emit finished(m_Name, m_TimeReceived);
    } else {
        auto *task(new NetworkTask());

        task->setTimeout(CTimeSync::HttpTimeout * 1000);
        task->setUrl(aHost);
        // По-хорошему, тут должен быть HEAD-запрос, но он почему-то не проходит аутентификацию на
        // proxy-сервере. Используем GET и извлекаем только Date header
        task->setType(NetworkTask::Get);
        task->setDataStream(new MemoryDataStream());

        connect(task, SIGNAL(onComplete()), this, SLOT(httpReplyReceived()));

        m_Network->addTask(task);
    }
}

//--------------------------------------------------------------------------------
void TimeSync::httpReplyReceived() {
    auto *task = dynamic_cast<NetworkTask *>(sender());

    m_TimeSyncHosts.remove(task->getUrl());

    // Проверка серверной даты и активация сигнала о её получении.
    QDateTime serverDate = task->getServerDate();
    if (serverDate.isValid()) {
        timeReceived(serverDate);
    } else {
        toLog(LogLevel::Error, QString("Get time failed. Error %1.").arg(task->errorString()));
    }
}
