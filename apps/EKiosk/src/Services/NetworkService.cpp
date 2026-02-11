/* @file Реализация менеджера сетевых соединений. */

#include "NetworkService.h"

#include <QtCore/QDateTime>
#include <QtCore/QRegularExpression>

#include <Common/ExceptionFilter.h>

#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/Watchdog/LineTypes.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/PaymentProcessor/Settings/Directory.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include "DatabaseUtils/IHardwareDatabaseUtils.h"
#include "Services/DatabaseService.h"
#include "Services/DeviceService.h"
#include "Services/ServiceCommon.h"
#include "Services/ServiceNames.h"
#include "Services/SettingsService.h"
#include "System/IApplication.h"

namespace PP = SDK::PaymentProcessor;

Q_DECLARE_METATYPE(SDK::PaymentProcessor::SConnection);
Q_DECLARE_METATYPE(bool *);

//---------------------------------------------------------------------------
namespace CNetworkService {
// Ожидание после неудачного подключения.
const int RestoreTimeout = 3 * 60 * 1000;

// Кол-во попыток перед перезагрузкой. (t = FailsBeforeReboot * RestoreTimeout мс.)
const int FailsBeforeReboot = 10;

// Время повторной проверки состояния соединения в случае dialup
const int CheckConnectionInterval = 3 * 60 * 1000;

// Интервал между обновлениями параметров модема.
const int UpdateStatusInterval = 12 * 60 * 60 * 1000;

// Интервал ожидания переустановки соединения, после сброса модема сторожом, с.
const int ReestablishInterval = 30;

// Время, в течении которого наблюдаются постоянные сетевые ошибки, приводящее к решению об обрыве
// связи (в минутах)
const int NetworkFailureTimeout = 3;
} // namespace CNetworkService

//---------------------------------------------------------------------------
NetworkService::NetworkService(IApplication *aApplication)
    : ILogable("Connection"), m_DeviceService(nullptr), m_EventService(nullptr),
      m_Connection(nullptr), m_Enabled(true), m_DontWatchConnection(false),
      m_Application(aApplication), m_Fails(0) {
    QObject::moveToThread(this);

    setObjectName(CServices::NetworkService);

    m_RestoreTimer.setSingleShot(true);
    m_RestoreTimer.moveToThread(this);
    connect(&m_RestoreTimer, SIGNAL(timeout()), SLOT(onConnectionLost()));

    m_ParametersUpdateTimer.moveToThread(this);
    m_ParametersUpdateTimer.setInterval(CNetworkService::UpdateStatusInterval);
    connect(&m_ParametersUpdateTimer, SIGNAL(timeout()), SLOT(updateModem_Parameters()));

    m_NetworkTaskManager.setLog(getLog());
    connect(&m_NetworkTaskManager,
            SIGNAL(networkTaskStatus(bool)),
            this,
            SLOT(onNetworkTaskStatus(bool)),
            Qt::QueuedConnection);

    qRegisterMetaType<SDK::PaymentProcessor::SConnection>();
    qRegisterMetaType<bool *>();
}

//---------------------------------------------------------------------------
NetworkService::~NetworkService() = default;

//---------------------------------------------------------------------------
bool NetworkService::initialize() {
    m_DeviceService = m_Application->getCore()->getDeviceService();

    m_EventService = m_Application->getCore()->getEventService();

    auto *terminalSettings =
        SettingsService::instance(m_Application)->getAdapter<PP::TerminalSettings>();

    m_DatabaseUtils =
        DatabaseService::instance(m_Application)->getDatabaseUtils<IHardwareDatabaseUtils>();

    // Активируем сохраненное в конфиге соединение.
    m_ConnectionSettings = terminalSettings->getConnection();

    // Создаем и запускаем поток сетевого сервиса.
    start();

    return true;
}

//------------------------------------------------------------------------------
void NetworkService::finishInitialize() {}

//---------------------------------------------------------------------------
bool NetworkService::canShutdown() {
    // Если нас спросили, можем ли выгрузится - это жжж не спроста.
    // Поэтому закрываем все текущие сетевые задачи.
    m_NetworkTaskManager.clearTasks();

    return true;
}

//---------------------------------------------------------------------------
bool NetworkService::shutdown() {
    m_Enabled = false;
    m_DontWatchConnection = true;

    // Останавливаем сетевой поток и дожидаемся его остановки.
    SafeStopServiceThread(&m_NetworkTaskManager, 10000, getLog());

    // Останавливаем сервисный поток и дожидаемся его остановки.
    SafeStopServiceThread(this, 3000, getLog());

    return true;
}

//---------------------------------------------------------------------------
QString NetworkService::getName() const {
    return CServices::NetworkService;
}

//---------------------------------------------------------------------------
QVariantMap NetworkService::getParameters() const {
    QVariantMap parameters;

    if (m_SignalLevel.is_initialized()) {
        parameters[PP::CServiceParameters::Networking::SignalLevel] = m_SignalLevel.get();
    }

    if (m_Balance.is_initialized()) {
        parameters[PP::CServiceParameters::Networking::SimBalance] = m_Balance.get();
    }

    if (m_Operator.is_initialized()) {
        parameters[PP::CServiceParameters::Networking::Provider] = m_Operator.get();
    }

    return parameters;
}

//---------------------------------------------------------------------------
void NetworkService::resetParameters(const QSet<QString> & /*aParameters*/) {}

//---------------------------------------------------------------------------
const QSet<QString> &NetworkService::getRequiredServices() const {
    static QSet<QString> requiredServices =
        QSet<QString>() << CServices::DeviceService << CServices::EventService
                        << CServices::DatabaseService << CServices::SettingsService;

    return requiredServices;
}

//---------------------------------------------------------------------------
bool NetworkService::getConnectionTemplate(const QString &aConnectionName,
                                           PP::SConnectionTemplate &aConnectionTemplate) const {
    auto *directory = SettingsService::instance(m_Application)->getAdapter<PP::Directory>();

    foreach (PP::SConnectionTemplate connectionTemplate, directory->getConnectionTemplates()) {
        if (connectionTemplate.name == aConnectionName) {
            aConnectionTemplate = connectionTemplate;
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------
NetworkTaskManager *NetworkService::getNetworkTaskManager() {
    return &m_NetworkTaskManager;
}

//---------------------------------------------------------------------------
bool NetworkService::openConnection(bool aWait) {
    QMetaObject::invokeMethod(
        this,
        "doConnect",
        aWait ? Qt::BlockingQueuedConnection : Qt::QueuedConnection,
        Q_ARG(const SDK::PaymentProcessor::SConnection &, m_ConnectionSettings));

    return true;
}

//---------------------------------------------------------------------------
bool NetworkService::closeConnection() {
    return QMetaObject::invokeMethod(this, "doDisconnect", Qt::BlockingQueuedConnection);
}

//---------------------------------------------------------------------------
bool NetworkService::isConnected(bool aUseCache) {
    try {
        return m_Connection ? m_Connection->isConnected(aUseCache) : false;
    } catch (const NetworkError &e) {
        toLog(LogLevel::Error, e.getMessage());
        if (e.getSeverity() == ESeverity::Critical) {
            toLog(LogLevel::Fatal, "Generating reboot event due to critical error.");
            m_EventService->sendEvent(PP::Event(PP::EEventType::Reboot));
        }

        QMutexLocker lock(&m_ErrorMutex);
        m_LastConnectionError = e.getMessage();

        return false;
    }
}

//---------------------------------------------------------------------------
void NetworkService::setConnection(const PP::SConnection &aConnection) {
    toLog(LogLevel::Normal, QString("Setting new connection '%1'.").arg(aConnection.name));

    // Страхуемся от изменений m_Connection
    PP::SConnection connectionSettings = getConnection();

    if (connectionSettings == aConnection) {
        toLog(LogLevel::Normal, "Already set up with an indentical connection.");
        return;
    }

    // Запоминаем, установлено или нет соединение.
    if (isConnected()) {
        // Разрываем старое соединение.
        QMetaObject::invokeMethod(this, "doDisconnect", Qt::QueuedConnection);
    }

    // Устанавливаем новое соединение.
    QMetaObject::invokeMethod(this,
                              "doConnect",
                              Qt::QueuedConnection,
                              Q_ARG(const SDK::PaymentProcessor::SConnection &, aConnection));
}

//---------------------------------------------------------------------------
PP::SConnection NetworkService::getConnection() const {
    return m_ConnectionSettings;
}

//---------------------------------------------------------------------------
bool NetworkService::testConnection() {
    bool result = false;
    QMetaObject::invokeMethod(
        this, "doTestConnection", Qt::BlockingQueuedConnection, Q_ARG(bool *, &result));
    return result;
}

//---------------------------------------------------------------------------
void NetworkService::doTestConnection(bool *aResult) {
    // Ожидаем установки соединения.
    if (!isConnected()) {
        doConnect(getConnection());
    }

    toLog(LogLevel::Normal, QString("Testing connection '%1'...").arg(m_Connection->getName()));

    try {
        *aResult = m_Connection->isConnected() && m_Connection->checkConnection();
    } catch (const NetworkError &e) {
        toLog(LogLevel::Error, e.getMessage());

        if (e.getSeverity() == ESeverity::Critical) {
            toLog(LogLevel::Fatal, "Generating reboot event due to critical error.");
            m_EventService->sendEvent(PP::Event(PP::EEventType::Reboot));
        }

        *aResult = false;

        QMutexLocker lock(&m_ErrorMutex);
        m_LastConnectionError = e.getMessage();
    }
}

//---------------------------------------------------------------------------
void NetworkService::updateModem_Parameters() {
    if (getConnection().type != EConnectionTypes::Dialup || !m_Enabled) {
        return;
    }

    toLog(LogLevel::Normal, "Updating GPRS connection parameters...");

    // Разрываем соединение для доступа к модему.
    doDisconnect();

    // Получаем параметры.
    SDK::Driver::IModem *modemDevice = prepareModem(getModem(), "");

    if (modemDevice) {
        m_Balance.reset();
        m_Operator.reset();
        m_SignalLevel.reset();

        PP::SConnectionTemplate connectionTemplate;

        if (getConnectionTemplate(getConnection().name, connectionTemplate)) {
            QString reply;

            // Выполняем USSD-запрос и извлекаем из него строку с информацией о балансе.
            if (modemDevice->processUSSD(connectionTemplate.balanceNumber, reply)) {
                QRegularExpression regExp(connectionTemplate.regExp);
                QRegularExpressionMatch match = regExp.match(reply);

                if (match.capturedStart() != -1) {
                    m_Balance = match.captured(0);
                    m_DatabaseUtils->setDeviceParam(
                        m_DeviceService->getDeviceConfigName(modemDevice),
                        PP::CDatabaseConstants::Parameters::BalanceLevel,
                        m_Balance.get());
                } else {
                    toLog(LogLevel::Error, QString("Failed to parse USSD reply: '%1'.").arg(reply));
                }
            } else {
                toLog(LogLevel::Error,
                      QString("Failed to send USSD: '%1'.").arg(connectionTemplate.balanceNumber));
            }
        } else {
            toLog(LogLevel::Error,
                  QString("Connection template '%1' not found. USSD request for balance undefined.")
                      .arg(getConnection().name));
        }

        QString operatorName;

        if (modemDevice->getOperator(operatorName)) {
            m_Operator = operatorName;
        } else {
            toLog(LogLevel::Error, "Failed to retrieve GSM operator info.");
            // В качестве имени оператора связи ставим имя соединения
            m_Operator = getConnection().name;
        }

        m_DatabaseUtils->setDeviceParam(m_DeviceService->getDeviceConfigName(modemDevice),
                                        PP::CDatabaseConstants::Parameters::ConnectionName,
                                        m_Operator.get());

        int signalLevel = 0;

        if (modemDevice->getSignalQuality(signalLevel)) {
            m_SignalLevel = signalLevel;
            m_DatabaseUtils->setDeviceParam(m_DeviceService->getDeviceConfigName(modemDevice),
                                            PP::CDatabaseConstants::Parameters::SignalLevel,
                                            m_SignalLevel.get());
        } else {
            toLog(LogLevel::Error, "Failed to retrieve signal level.");
        }

        m_DatabaseUtils->setDeviceParam(m_DeviceService->getDeviceConfigName(modemDevice),
                                        PP::CDatabaseConstants::Parameters::LastCheckBalanceTime,
                                        QDateTime::currentDateTime());

        if (m_Balance.is_initialized()) {
            toLog(LogLevel::Normal, QString("Balance is: %1").arg(m_Balance.get()));
        } else {
            toLog(LogLevel::Error, "Get balance error.");
        }

        QString modemInfo;

        if (modemDevice->getInfo(modemInfo)) {
            toLog(LogLevel::Normal, QString("Modem info: %1").arg(modemInfo));
            m_DatabaseUtils->setDeviceParam(m_DeviceService->getDeviceConfigName(modemDevice),
                                            PP::CDatabaseConstants::Parameters::DeviceInfo,
                                            modemInfo);
        }

        SDK::Driver::IModem::TMessages messages;

        if (modemDevice->takeMessages(messages)) {
            foreach (auto sms, messages) {
                toLog(LogLevel::Normal,
                      QString("SMS [%1] at %2: %3.")
                          .arg(sms.from)
                          .arg(sms.date.toString())
                          .arg(sms.text));
            }
        }
    } else {
        toLog(LogLevel::Error, "Failed to retrieve modem parameters.");
    }

    // Восстанавливаем соединение.
    QMetaObject::invokeMethod(
        this,
        "doConnect",
        Qt::QueuedConnection,
        Q_ARG(const SDK::PaymentProcessor::SConnection &, m_ConnectionSettings));
}

//---------------------------------------------------------------------------
void NetworkService::doConnect(const SDK::PaymentProcessor::SConnection &aConnection) {
    if (!m_Enabled) {
        return;
    }

    try {
        toLog(LogLevel::Normal,
              QString("Attempt %2: establishing connection '%1'...")
                  .arg(aConnection.name)
                  .arg(m_Fails));

        m_Connection =
            QSharedPointer<IConnection>(IConnection::create(aConnection.name,
                                                            aConnection.type,
                                                            &m_NetworkTaskManager,
                                                            ILog::getInstance("Connection")));

        m_Connection->setCheckPeriod(aConnection.checkInterval);

        // Всегда читаем список серверов для проверки соединения из конфигов.
        auto *settings =
            SettingsService::instance(m_Application)->getAdapter<PP::TerminalSettings>();
        m_Connection->setCheckHosts(settings->getCheckHosts());
        connect(m_Connection.data(),
                SIGNAL(connectionLost()),
                SLOT(onConnectionLost()),
                Qt::QueuedConnection);
        connect(m_Connection.data(),
                SIGNAL(connectionAlive()),
                SLOT(onConnectionAlive()),
                Qt::QueuedConnection);

        m_ConnectionSettings = aConnection;

        // Попытка поднять соединение.
        if (!isConnected()) {
            if (aConnection.type == EConnectionTypes::Dialup) {
                if (!prepareModem(getModem(), aConnection.name)) {
                    toLog(LogLevel::Warning, "Failed to initialize modem device.");
                }
            }
        }

        // Открываем соединение в любом случае
        m_Connection->open();

        if (isConnected()) {
            // Устанавливаем прокси-сервер.
            if (getConnection().type == EConnectionTypes::Dialup) {
                toLog(LogLevel::Normal, "Proxy disabled.");
                m_NetworkTaskManager.setProxy(QNetworkProxy::NoProxy);
            } else {
                toLog(LogLevel::Normal,
                      QString("Using proxy: host = %1, port = %2.")
                          .arg(getConnection().proxy.hostName())
                          .arg(getConnection().proxy.port()));
                m_NetworkTaskManager.setProxy(getConnection().proxy);
            }

            // Посылаем сообщение о том, что связь восстановлена.
            m_EventService->sendEvent(PP::Event(PP::EEventType::ConnectionEstablished));

            toLog(LogLevel::Normal, QString("Connected to '%1'.").arg(aConnection.name));
        } else {
            toLog(LogLevel::Error, QString("Failed to connect to '%1'.").arg(aConnection.name));
        }

        // TODO #29565 - проверяем статус модема, и в случае ошибок выставляем ему статус OK -
        // Connection enstablished
        if (isConnected() && getConnection().type == EConnectionTypes::Dialup) {
            auto *modem = getModem();
            if (modem) {
                DeviceService::instance(m_Application)
                    ->overwriteDeviceStatus(modem, SDK::Driver::EWarningLevel::OK, "Connected", 0);
            }
        }
    } catch (const NetworkError &e) {
        toLog(LogLevel::Error, e.getMessage());

        if (e.getSeverity() == ESeverity::Critical) {
            toLog(LogLevel::Fatal,
                  "ConnectionManager: generating reboot event due to critical error.");

            m_EventService->sendEvent(PP::Event(PP::EEventType::Reboot));
        }

        QMutexLocker lock(&m_ErrorMutex);
        m_LastConnectionError = e.getMessage();
    }

    if (!isConnected()) {
        // Не получилось, увеличиваем счётчики попыток
        ++m_Fails;

        // Ожидание перед следующим подключением линейно увеличивается
        m_RestoreTimer.start(CNetworkService::RestoreTimeout);
    } else {
        m_RestoreTimer.stop();
    }
}

//---------------------------------------------------------------------------
void NetworkService::checkConnection() {
    try {
        if (!isConnected()) {
            toLog(LogLevel::Warning, "Forced connection attempt.");

            QMetaObject::invokeMethod(
                this,
                "doConnect",
                Qt::QueuedConnection,
                Q_ARG(const SDK::PaymentProcessor::SConnection &, m_ConnectionSettings));
        }
    } catch (const NetworkError &e) {
        toLog(LogLevel::Error, e.getMessage());

        if (e.getSeverity() == ESeverity::Critical) {
            toLog(LogLevel::Fatal, "Generating reboot event due to critical error.");
            m_EventService->sendEvent(PP::Event(PP::EEventType::Reboot));
        }

        QMutexLocker lock(&m_ErrorMutex);
        m_LastConnectionError = e.getMessage();
    }
}

//---------------------------------------------------------------------------
void NetworkService::run() {
    try {
        toLog(LogLevel::Normal, "NetworkService: Service thread started.");

        // Запускаем таймер на проверку параметров модема.
        m_ParametersUpdateTimer.start();

        SDK::Driver::IModem *modem = getModem();

        if (getConnection().type == EConnectionTypes::Dialup && (modem != nullptr)) {
            modem->subscribe(
                SDK::Driver::IDevice::InitializedSignal, this, SLOT(onModem_Initialized()));

            // если модем не инициализируется за 3 минуты запустим соединение принудительно
            QTimer::singleShot(
                CNetworkService::CheckConnectionInterval, this, SLOT(checkConnection()));
        } else {
            doConnect(getConnection());
        }

        QThread::exec();

        if (isConnected()) {
            doDisconnect();
        }
    } catch (...) {
        EXCEPTION_FILTER_NO_THROW(getLog());
    }

    m_EventService->sendEvent(PP::Event(PP::EEventType::ConnectionLost));
}

//---------------------------------------------------------------------------
void NetworkService::onModem_Initialized() {
    // Запускаем первую проверку параметров модема
    QMetaObject::invokeMethod(this, "updateModem_Parameters", Qt::QueuedConnection);
}

//---------------------------------------------------------------------------
void NetworkService::onConnectionAlive() {
    // В случае успешной проверки связи - сбрасываем счетчик обрывов
    m_Fails = 0;
}

//---------------------------------------------------------------------------
void NetworkService::onConnectionLost() {
    toLog(LogLevel::Warning, "Connection lost.");

    if (m_DontWatchConnection || !m_Enabled) {
        return;
    }

    // Увеличиваем счётчик обрыва связи
    ++m_Fails;

    // Посылаем сообщение о том, что связи сейчас нет
    m_EventService->sendEvent(PP::Event(PP::EEventType::ConnectionLost));

    doDisconnect();

    // Возможно, нужно перегузить терминал.
    if (m_Fails >= CNetworkService::FailsBeforeReboot) {
        toLog(LogLevel::Warning,
              QString("Generating system reboot event after %1 unsuccessful tries to establish "
                      "connection.")
                  .arg(m_Fails));

        // WARNING: если WatchService не отработал (standalone-режим), соединение не будет
        // восстанавливаться.
        m_EventService->sendEvent(PP::Event(PP::EEventType::Reboot));
        return;
    }

    // TODO #29565 - проверяем статус модема, и в случае OK выставляем ему статус Warning -
    // Disconnected
    if (getConnection().type == EConnectionTypes::Dialup) {
        auto *modem = getModem();
        if (modem) {
            DeviceService::instance(m_Application)
                ->overwriteDeviceStatus(
                    modem, SDK::Driver::EWarningLevel::Warning, "Disconnected", 0);
        }
    }

    // Или сбросить модем сторожевиком.
    if (getConnection().type == EConnectionTypes::Dialup) {
        // Сбрасываем модем посредством сторожа.
        auto *settings =
            SettingsService::instance(m_Application)->getAdapter<PP::TerminalSettings>();

        QStringList watchdogConfigName =
            settings->getDeviceList().filter(SDK::Driver::CComponents::Watchdog);

        SDK::Driver::IWatchdog *watchdogDevice = nullptr;

        if (!watchdogConfigName.empty()) {
            watchdogDevice = dynamic_cast<SDK::Driver::IWatchdog *>(
                m_DeviceService->acquireDevice(watchdogConfigName.first()));
        }

        if (watchdogDevice) {
            toLog(LogLevel::Warning, "Resetting modem with watchcdog.");

            // Сбросываем питание модема.
            if (watchdogDevice->reset(SDK::Driver::LineTypes::Modem)) {
                // Ждем, пока модем оживет после сброса питания.
                sleep(CNetworkService::ReestablishInterval);
            } else {
                toLog(LogLevel::Error, "Failed to reset modem by watchcdog.");
            }
        } else {
            toLog(LogLevel::Error, "Failed to acquire watchdog device. Modem reset failed.");
        }
    }

    QMetaObject::invokeMethod(
        this,
        "doConnect",
        Qt::QueuedConnection,
        Q_ARG(const SDK::PaymentProcessor::SConnection &, m_ConnectionSettings));
}

//---------------------------------------------------------------------------
SDK::Driver::IModem *NetworkService::getModem() {
    // Запрашиваем модем.
    auto *settings = SettingsService::instance(m_Application)->getAdapter<PP::TerminalSettings>();

    QStringList modemConfigName = settings->getDeviceList().filter(SDK::Driver::CComponents::Modem);
    SDK::Driver::IModem *modemDevice =
        !modemConfigName.empty() ? dynamic_cast<SDK::Driver::IModem *>(
                                       m_DeviceService->acquireDevice(modemConfigName.first()))
                                 : nullptr;

    if (!modemDevice) {
        toLog(LogLevel::Error, "Modem is not present.");
    }

    return modemDevice;
}

//---------------------------------------------------------------------------
SDK::Driver::IModem *NetworkService::prepareModem(SDK::Driver::IModem *aModemDevice,
                                                  const QString &aConnectionName) {
    if (aModemDevice) {
        // Сбрасываем модем.
        if (!aModemDevice->reset()) {
            toLog(LogLevel::Error, "Failed to reset modem.");
        }

        PP::SConnectionTemplate connectionTemplate;

        // Инициализируем модем.
        if (getConnectionTemplate(aConnectionName, connectionTemplate)) {
            if (!aModemDevice->setInitString(connectionTemplate.initString)) {
                toLog(LogLevel::Error,
                      QString("Failed to set modem initialization string: %1.")
                          .arg(connectionTemplate.initString));
            }
        }
    }

    return aModemDevice;
}

//---------------------------------------------------------------------------
bool NetworkService::doDisconnect() {
    try {
        if (isConnected()) {
            m_Connection->close();
        }

        toLog(LogLevel::Normal, QString("Disconnected from '%1'.").arg(getConnection().name));

        return true;
    } catch (const NetworkError &e) {
        toLog(LogLevel::Error, e.getMessage());

        if (e.getSeverity() == ESeverity::Critical) {
            toLog(LogLevel::Fatal, "Generating reboot event due to critical error.");
            m_EventService->sendEvent(PP::Event(PP::EEventType::Reboot));
        }

        QMutexLocker lock(&m_ErrorMutex);
        m_LastConnectionError = e.getMessage();
    }

    return false;
}

//---------------------------------------------------------------------------
QString NetworkService::getLastConnectionError() const {
    QMutexLocker lock(&m_ErrorMutex);
    return m_LastConnectionError;
}

//---------------------------------------------------------------------------
void NetworkService::setUserAgent(const QString aUserAgent) {
    m_NetworkTaskManager.setUserAgent(aUserAgent);
}

//---------------------------------------------------------------------------
QString NetworkService::getUserAgent() const {
    return m_NetworkTaskManager.getUserAgent();
}

//---------------------------------------------------------------------------
void NetworkService::onNetworkTaskStatus(bool aFailure) {
    if (!aFailure) {
        // при успешном обращении сбрасываем метку первой ошибки
        m_NetworkTaskFailureStamp = QDateTime();
    } else if (m_NetworkTaskFailureStamp.isNull()) {
        // первая ошибка
        m_NetworkTaskFailureStamp = QDateTime::currentDateTime();
    } else {
        const qint64 failureMinutes =
            qAbs(m_NetworkTaskFailureStamp.secsTo(QDateTime::currentDateTime())) / 60;

        if (failureMinutes > CNetworkService::NetworkFailureTimeout) {
            toLog(LogLevel::Error,
                  QString("Errors network calls for %1 minutes. Connection lost.")
                      .arg(CNetworkService::NetworkFailureTimeout));

            m_NetworkTaskFailureStamp = QDateTime();

            QMetaObject::invokeMethod(this, "onConnectionLost", Qt::QueuedConnection);
        }

        QMetaObject::invokeMethod(this, "onConnectionLost", Qt::QueuedConnection);
    }
}

//---------------------------------------------------------------------------
