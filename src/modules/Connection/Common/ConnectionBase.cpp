/* @file Реализация базового функционала сетевого соединения. */

#include "ConnectionBase.h"

#include <QtCore/QStringList>

#include <Common/ScopedPointerLaterDeleter.h>

#include <NetworkTaskManager/MemoryDataStream.h>
#include <NetworkTaskManager/NetworkTask.h>
#include <NetworkTaskManager/NetworkTaskManager.h>

namespace CConnection {
/// Период проверки статуса соединения.
const int DefaultCheckPeriod = 60 * 1000; // 1 минутa

/// Период пинга соединения.
const int DefaultPingPeriod = 15 * 60 * 1000; // 15 минут

/// Таймаут запроса проверки соединения.
const int PingTimeout = 30 * 1000; // 30 секунд

/// Хост по умолчанию для проверки соединения.
const QString DefaultCheckHost = "http://mon.humo.tj:80/ping";

/// Строка ответа по умолчанию для проверки соединения.
const QString DefaultCheckResponse = "";
} // namespace CConnection

//--------------------------------------------------------------------------------
ConnectionBase::ConnectionBase(const QString &aName, NetworkTaskManager *aNetwork, ILog *aLog)
    : m_Network(aNetwork), m_Name(aName), m_Connected(false), m_CheckCount(0), m_Watch(false),
      m_Log(aLog) {
    // Таймер будет взводится заново после каждой удачной проверки
    m_CheckTimer.setSingleShot(true);
    m_CheckTimer.setInterval(CConnection::DefaultCheckPeriod);
    QObject::connect(&m_CheckTimer, SIGNAL(timeout()), this, SLOT(onCheckTimeout()));

    m_CheckHosts << IConnection::CheckUrl(QUrl(CConnection::DefaultCheckHost),
                                          CConnection::DefaultCheckResponse);
}

//--------------------------------------------------------------------------------
void ConnectionBase::open(bool aWatch) noexcept(false) {
    toLog(LogLevel::Normal, QString("*").repeated(40));
    toLog(LogLevel::Normal, QString("Connection:     %1").arg(getName()));
    toLog(LogLevel::Normal,
          QString("Type:           %1").arg(EConnectionTypes::getConnectionTypeName(getType())));

    if (aWatch) {
        toLog(LogLevel::Normal,
              QString("Check interval: %1 sec").arg(m_CheckTimer.interval() / 1000));
        toLog(LogLevel::Normal,
              QString("Ping interval: %1 sec").arg(m_CheckTimer.interval() * m_PingPeriod / 1000));
    } else {
        toLog(LogLevel::Normal, "Check interval: uncontrolled connection");
    }

    toLog(LogLevel::Normal, QString("*").repeated(40));

    if (!isConnected(false)) {
        doConnect();
    } else {
        toLog(LogLevel::Normal, "Already connected.");
    }

    m_Watch = aWatch;
    m_CheckCount = 0;
    m_Connected = true;

    // Если нужно наблюдение за соединением
    if (m_Watch) {
        toLog(LogLevel::Normal, "Check timer START.");
        m_CheckTimer.start();

        // пропингуем сервер на следующем шаге проверки
        m_CheckCount = m_PingPeriod - 1;
    }
}

//--------------------------------------------------------------------------------
void ConnectionBase::close() noexcept(false) {
    toLog(LogLevel::Debug, "Check timer STOP.");
    m_CheckTimer.stop();

    if (isConnected(true)) {
        doDisconnect();
    }

    m_Connected = false;
}

//--------------------------------------------------------------------------------
QString ConnectionBase::getName() const {
    return m_Name;
}

//--------------------------------------------------------------------------------
void ConnectionBase::setCheckPeriod(int aMinutes) {
    m_PingPeriod = aMinutes;
}

//--------------------------------------------------------------------------------
bool ConnectionBase::isConnected(bool aUseCache) noexcept(false) {
    if (!aUseCache) {
        m_Connected = doIsConnected();
    }

    return m_Connected;
}

//--------------------------------------------------------------------------------
bool ConnectionBase::checkConnection(const IConnection::CheckUrl &aHost) noexcept(false) {
    QList<CheckUrl> hosts;

    if (aHost.first.isValid() && !aHost.first.isEmpty()) {
        hosts << aHost;
    } else {
        hosts = m_CheckHosts;
    }

    if (hosts.isEmpty()) {
        toLog(LogLevel::Error, QString("Cannot check connection, no hosts specified."));
        return true;
    }

    // Для каждого хоста из списка или до первой удачной проверки
    foreach (auto host, hosts) {
        if (doCheckConnection(host)) {
            emit connectionAlive();

            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------
void ConnectionBase::setCheckHosts(const QList<IConnection::CheckUrl> &aHosts) {
    if (aHosts.isEmpty()) {
        toLog(LogLevel::Warning, QString("No check hosts specified, using previous/default ones."));
    } else {
        m_CheckHosts = aHosts;
    }
}

//--------------------------------------------------------------------------------
void ConnectionBase::onCheckTimeout() {
    ++m_CheckCount;

    toLog(LogLevel::Debug, QString("Check timeout. count: %1.").arg(m_CheckCount));

    try {
        // Проверяем состояние соединения через апи ОС и делаем http запрос если
        // подошло время
        if (isConnected(false) && m_CheckCount >= m_PingPeriod) {
            m_CheckCount = 0;
            m_Connected = checkConnection();
        }

        if (isConnected(true)) {
            toLog(LogLevel::Debug, "Check timer START.");
            m_CheckTimer.start();
        } else {
            emit connectionLost();
        }
    } catch (const NetworkError &e) {
        toLog(LogLevel::Error, e.getMessage());

        emit connectionLost();
    }
}

//--------------------------------------------------------------------------------
bool ConnectionBase::httpCheckMethod(const IConnection::CheckUrl &aHost) {
    toLog(LogLevel::Normal, QString("Checking connection on %1...").arg(aHost.first.toString()));

    if (!m_Network) {
        toLog(LogLevel::Error, "Failed to check connection. Network interface is not specified.");

        return false;
    }

    QScopedPointer<NetworkTask, ScopedPointerLaterDeleter<NetworkTask>> task(new NetworkTask());

    task->setTimeout(CConnection::PingTimeout);
    task->setUrl(aHost.first);
    // По-хорошему, тут должен быть HEAD-запрос, но он почему-то не проходит
    // аутентификацию на прокси-сервере.
    task->setType(NetworkTask::Get);
    task->setDataStream(new MemoryDataStream());

    m_Network->addTask(task.data());

    task->waitForFinished();

    QByteArray answer = task->getDataStream()->takeAll();

    auto traceLog = [&]() {
        toLog(LogLevel::Trace,
              QString("error:%1 http_code:%2").arg(task->getError()).arg(task->getHttpError()));

        QStringList response;
        QMapIterator<QByteArray, QByteArray> i(task->getResponseHeader());
        while (i.hasNext()) {
            i.next();
            response << QString("%1: %2")
                            .arg(QString::fromLatin1(i.key()))
                            .arg(QString::fromLatin1(i.value()));
        }

        toLog(LogLevel::Trace,
              QString("HEADER:\n%1\nBODY:\n%2")
                  .arg(response.join("\n"))
                  .arg(QString::fromLatin1(answer.left(80))));
    };

    if (task->getError() != NetworkTask::NoError) {
        toLog(LogLevel::Error,
              QString("Connection check failed. Error %1.").arg(task->errorString()));

        traceLog();

        return false;
    }

    if (!aHost.second.isEmpty() && !answer.contains(aHost.second.toLatin1())) {
        toLog(LogLevel::Error,
              QString("Server answer verify failed '%1'.\nServer response: '%2'.")
                  .arg(aHost.second)
                  .arg(QString::fromUtf8(answer).left(1024)));

        traceLog();

        return false;
    }

    toLog(LogLevel::Normal, "Connection check ok.");

    return true;
}

//----------------------------------------------------------------------------
void ConnectionBase::toLog(LogLevel::Enum aLevel, const QString &aMessage) const {
    m_Log->write(aLevel, aMessage);
}

//----------------------------------------------------------------------------

IConnection *IConnection::create(const QString &aName,
                                 EConnectionTypes::Enum aType,
                                 NetworkTaskManager *aNetwork,
                                 ILog *aLog) {
#ifdef Q_OS_WIN32
    // Windows-specific implementation is in Win32/src/Common.cpp
    return nullptr; // This will be overridden by the Windows implementation
#else
    // Non-Windows platforms don't support dial-up connections
    Q_UNUSED(aName)
    Q_UNUSED(aType)
    Q_UNUSED(aNetwork)
    Q_UNUSED(aLog)
    return nullptr;
#endif
}

//----------------------------------------------------------------------------
