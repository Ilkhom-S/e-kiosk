/* @file Менеджер, загружающий клиенты мониторинга. */

// Stl

#include "Services/RemoteService.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QMutexLocker>

#include <SDK/Drivers/Components.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/DatabaseConstants.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <WatchServiceClient/Constants.h>
#include <functional>
#include <utility>

#include "DatabaseUtils/IHardwareDatabaseUtils.h"
#include "Services/CryptService.h"
#include "Services/DatabaseService.h"
#include "Services/DeviceService.h"
#include "Services/EventService.h"
#include "Services/PaymentService.h"
#include "Services/PluginService.h"
#include "Services/ServiceNames.h"
#include "Services/SettingsService.h"
#include "Services/TerminalService.h"

namespace CRemoteService {
// Лог сервиса
const QString LogName = "Monitoring";

// Файл конфигурации сервиса
const QString ConfigFileName = "/update/update_commands.ini";

// Номер последней команды обновления, зарегистрированной у модуля обновления.
const char LastMonitoringCommand[] = "last_monitoring_command";

// Список отложенных команд перезагрузки и выключения. Их статусы будут обновлены при следующем
// включении клиента.
const char QueuedRebootCommands[] = "queued_reboot_commands";

// Номер текущей команды регенерации ключей
const char GenKeyCommandId[] = "gen_key_commands";

// Разделитель значений в свойстве QueuedRebootCommands.
const char QueuedRebootCommandDelimeter[] = "|";

const int UpdateReportCheckTimeout = 5 * 1000;
const int UpdateReportFirstCheckInterval = 1 * 60 * 1000;
const int UpdateReportCheckInterval = 10 * 60 * 1000;
const int CommandCheckInterval = 1 * 60 * 1000;
const int MaxUpdateCommandLifetime = 60 * 30;

const int GeneratedKeyId = 100;

const char DateTimeFormat[] = "yyyy.MM.dd hh:mm:ss";
} // namespace CRemoteService

//---------------------------------------------------------------------------
struct CommandReport {
    QString filePath;

    int id{0};
    SDK::PaymentProcessor::IRemoteService::EStatus status{
        SDK::PaymentProcessor::IRemoteService::OK};
    QDateTime lastUpdate;
    QVariant description;
    QVariant progress;

    CommandReport(QString aFilePath) : filePath(std::move(aFilePath)) {}

    /// Удалить файл отчета
    void remove() const { QFile::remove(filePath); }

    /// Проверка что команда ещё выполняется, а не "подвисла"
    bool isAlive() const {
        return lastUpdate.addSecs(qint64(60) * 10) > QDateTime::currentDateTime();
    }
};

//---------------------------------------------------------------------------
RemoteService *RemoteService::instance(IApplication *aApplication) {
    return dynamic_cast<RemoteService *>(
        aApplication->getCore()->getService(CServices::RemoteService));
}

//---------------------------------------------------------------------------
RemoteService::RemoteService(IApplication *aApplication)
    : ILogable(CRemoteService::LogName), m_Application(aApplication), m_Database(nullptr),
      m_LastCommand(0), m_GenerateKeyCommand(0),
      m_Settings(IApplication::getWorkingDirectory() + CRemoteService::ConfigFileName,
                 QSettings::IniFormat) {
    // Создаем 5сек таймер отложенной проверки состояния файлов отчета
    m_CheckUpdateReportsTimer.setSingleShot(true);
    m_CheckUpdateReportsTimer.setInterval(CRemoteService::UpdateReportCheckTimeout);
    connect(&m_CheckUpdateReportsTimer, SIGNAL(timeout()), this, SLOT(onCheckUpdateReports()));
}

//---------------------------------------------------------------------------
RemoteService::~RemoteService() = default;

//---------------------------------------------------------------------------
bool RemoteService::initialize() {
    m_Database =
        DatabaseService::instance(m_Application)->getDatabaseUtils<IHardwareDatabaseUtils>();

    QVariant lastCommand =
        m_Database->getDeviceParam(SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal,
                                   CRemoteService::LastMonitoringCommand);
    if (!lastCommand.isValid()) {
        m_LastCommand = 0;
    } else {
        m_LastCommand = lastCommand.toInt();
    }

    connect(PaymentService::instance(m_Application),
            SIGNAL(paymentCommandComplete(int, EPaymentCommandResult::Enum)),
            SLOT(onPaymentCommandComplete(int, EPaymentCommandResult::Enum)),
            Qt::QueuedConnection);

    QStringList remotes =
        PluginService::instance(m_Application)
            ->getPluginLoader()
            ->getPluginList(QRegularExpression(
                QString("%1\\.%2\\..*").arg(PPSDK::Application, PPSDK::CComponents::RemoteClient)));

    foreach (const QString &path, remotes) {
        SDK::Plugin::IPlugin *plugin =
            PluginService::instance(m_Application)->getPluginLoader()->createPlugin(path);
        if (plugin) {
            auto *client = dynamic_cast<PPSDK::IRemoteClient *>(plugin);
            if (client) {
                m_MonitoringClients << client;
            } else {
                PluginService::instance(m_Application)->getPluginLoader()->destroyPlugin(plugin);
            }
        }
    }

    auto *deviceService = DeviceService::instance(m_Application);

    connect(
        deviceService, SIGNAL(configurationUpdated()), this, SLOT(onDeviceConfigurationUpdated()));
    connect(deviceService,
            SIGNAL(deviceStatusChanged(
                const QString &, SDK::Driver::EWarningLevel::Enum, const QString &, int)),
            this,
            SLOT(onDeviceStatusChanged(const QString &)),
            Qt::QueuedConnection);

    return true;
}

//------------------------------------------------------------------------------
void RemoteService::onDeviceConfigurationUpdated() {
    foreach (auto client, m_MonitoringClients) {
        client->useCapability(PPSDK::IRemoteClient::DeviceConfigurationUpdated);
    }
}

//------------------------------------------------------------------------------
void RemoteService::finishInitialize() {
    restoreCommandQueue();

    foreach (auto client, m_MonitoringClients) {
        client->enable();
    }

    if (m_Database) {
        bool ok = true;
        m_GenerateKeyCommand =
            m_Database
                ->getDeviceParam(SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal,
                                 CRemoteService::GenKeyCommandId)
                .toInt(&ok);
        m_GenerateKeyCommand = ok ? m_GenerateKeyCommand : 0;

        QTimer::singleShot(
            CRemoteService::UpdateReportFirstCheckInterval, this, SLOT(onCheckUpdateReports()));
        QTimer::singleShot(
            CRemoteService::CommandCheckInterval, this, SLOT(onCheckQueuedRebootCommands()));

        QDir(IApplication::getWorkingDirectory()).mkpath("update");

        restartUpdateWatcher();

        // запускаем независимый таймер, отслеживающий состояние команд обновления
        startTimer(CRemoteService::UpdateReportCheckInterval);
    }

    if (!m_ScreenShotsCommands.isEmpty()) {
        QTimer::singleShot(CRemoteService::CommandCheckInterval, this, SLOT(doScreenshotCommand()));
    }
}

//---------------------------------------------------------------------------
bool RemoteService::canShutdown() {
    return true;
}

//---------------------------------------------------------------------------
bool RemoteService::shutdown() {
    saveCommandQueue();

    if (m_GenerateKeyFuture.isRunning()) {
        m_GenerateKeyFuture.waitForFinished();
    }

    disconnect(PaymentService::instance(m_Application),
               SIGNAL(paymentCommandComplete(int, EPaymentCommandResult::Enum)),
               this,
               SLOT(onPaymentCommandComplete(int, EPaymentCommandResult::Enum)));

    while (!m_MonitoringClients.isEmpty()) {
        m_MonitoringClients.first()->disable();

        PluginService::instance(m_Application)
            ->getPluginLoader()
            ->destroyPlugin(dynamic_cast<SDK::Plugin::IPlugin *>(m_MonitoringClients.first()));

        m_MonitoringClients.takeFirst();
    }

    m_CheckUpdateReportsTimer.stop();
    m_Database->setDeviceParam(
        SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal,
        CRemoteService::QueuedRebootCommands,
        m_QueuedRebootCommands.join(CRemoteService::QueuedRebootCommandDelimeter));

    return true;
}

//---------------------------------------------------------------------------
QString RemoteService::getName() const {
    return CServices::RemoteService;
}

//---------------------------------------------------------------------------
const QSet<QString> &RemoteService::getRequiredServices() const {
    // TODO: пересмотреть зависимости
    static QSet<QString> requiredServices =
        QSet<QString>() << CServices::SettingsService << CServices::EventService
                        << CServices::PluginService << CServices::DatabaseService
                        << CServices::PaymentService << CServices::DeviceService;

    return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap RemoteService::getParameters() const {
    return {};
}

//---------------------------------------------------------------------------
void RemoteService::resetParameters(const QSet<QString> & /*aParameters*/) {}

//---------------------------------------------------------------------------
int RemoteService::increaseLastCommandID() {
    ++m_LastCommand;

    m_Database->setDeviceParam(SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal,
                               CRemoteService::LastMonitoringCommand,
                               m_LastCommand);

    return m_LastCommand;
}

//---------------------------------------------------------------------------
int RemoteService::executeCommand(PPSDK::EEventType::Enum aEvent) {
    QMutexLocker lock(&m_CommandMutex);

    int command = increaseLastCommandID();

    QMetaObject::invokeMethod(
        this, "doExecuteCommand", Qt::QueuedConnection, Q_ARG(int, command), Q_ARG(int, aEvent));

    return command;
}

//---------------------------------------------------------------------------
void RemoteService::doExecuteCommand(int aComandId, int aEvent) {
    if ((aEvent == PPSDK::EEventType::Reboot) || (aEvent == PPSDK::EEventType::Shutdown)) {
        if (!allowRestart()) {
            emit commandStatusChanged(aComandId, Error, QVariantMap());

            return;
        }

        m_QueuedRebootCommands << QString::number(aComandId);
    } else if (aEvent == PPSDK::EEventType::Restart) {
        m_QueuedRebootCommands << QString::number(aComandId);
    } else {
        emit commandStatusChanged(aComandId, OK, QVariantMap());
    }

    EventService::instance(m_Application)->sendEvent(SDK::PaymentProcessor::Event(aEvent));
}

//---------------------------------------------------------------------------
int RemoteService::registerLockCommand() {
    return executeCommand(PPSDK::EEventType::TerminalLock);
}

//---------------------------------------------------------------------------
int RemoteService::registerUnlockCommand() {
    return executeCommand(PPSDK::EEventType::TerminalUnlock);
}

//---------------------------------------------------------------------------
int RemoteService::registerRebootCommand() {
    return executeCommand(PPSDK::EEventType::Reboot);
}

//---------------------------------------------------------------------------
int RemoteService::registerRestartCommand() {
    return executeCommand(PPSDK::EEventType::Restart);
}

//---------------------------------------------------------------------------
int RemoteService::registerShutdownCommand() {
    return executeCommand(PPSDK::EEventType::Shutdown);
}

//---------------------------------------------------------------------------
int RemoteService::registerAnyCommand() {
    return increaseLastCommandID();
}

//---------------------------------------------------------------------------
int RemoteService::registerPaymentCommand(EPaymentOperation aOperation,
                                          const QString &aInitialSession,
                                          const QVariantMap &aParameters) {
    QMutexLocker lock(&m_CommandMutex);

    if (m_GenerateKeyCommand != 0) {
        return 0;
    }

    int paymentCommand =
        aOperation == Remove
            ? PaymentService::instance(m_Application)->registerRemovePaymentCommand(aInitialSession)
            : PaymentService::instance(m_Application)
                  ->registerForcePaymentCommand(aInitialSession, aParameters);

    if (paymentCommand == 0) {
        return 0;
    }

    int command = increaseLastCommandID();

    m_PaymentCommands.insert(paymentCommand, command);

    return command;
}

//---------------------------------------------------------------------------
void RemoteService::onPaymentCommandComplete(int aID, EPaymentCommandResult::Enum aError) {
    int status = Error;
    switch (aError) {
    case EPaymentCommandResult::OK:
        status = OK;
        break;

    case EPaymentCommandResult::NotFound:
        status = PaymentNotFound;
        break;

    default:
        status = Error;
    }

    emit commandStatusChanged(m_PaymentCommands[aID], status, QVariantMap());

    m_PaymentCommands.remove(aID);
}

//---------------------------------------------------------------------------
bool RemoteService::allowUpdateCommand() {
    return m_UpdateCommands.isEmpty() && m_GenerateKeyCommand == 0 &&
           m_QueuedRebootCommands.isEmpty();
}

//---------------------------------------------------------------------------
bool RemoteService::allowRestart() {
    if (!m_UpdateCommands.isEmpty()) {
        // ускорение проверки статуса команды получения конфигурации
        onCheckUpdateReports();
    }

    foreach (auto command, m_UpdateCommands.values()) {
        if (command.type == FirmwareUpload) {
            toLog(LogLevel::Error, "Deny restart/shutdown because FirmwareUpload processed.");

            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------
int RemoteService::registerUpdateCommand(EUpdateType aType,
                                         const QUrl &aConfigUrl,
                                         const QUrl &aUpdateUrl,
                                         const QString &aComponents) {
    UpdateCommand command;

    command.ID = increaseLastCommandID();
    command.status = IRemoteService::Waiting;
    command.type = aType;
    command.configUrl = aConfigUrl;
    command.updateUrl = aUpdateUrl;

    if (!aComponents.trimmed().isEmpty()) {
        command.parameters = aComponents.trimmed().split("#");
    }

    if (!m_UpdateCommands.isEmpty()) {
        // ускорение проверки статуса команды получения конфигурации
        onCheckUpdateReports();
    }

    if (!allowUpdateCommand()) {
        QMutexLocker lock(&m_CommandMutex);

        toLog(LogLevel::Normal,
              QString(
                  "Update command added to the queue. ID:%1 type:%2 url:%3 url2:%4 parameters:%5.")
                  .arg(command.ID)
                  .arg(command.type)
                  .arg(command.configUrl.toString())
                  .arg(command.updateUrl.toString())
                  .arg(command.parameters.join("#")));

        m_UpdateCommands.insert(command.ID, command);

        saveCommandQueue();

        return command.ID;
    }

    return startUpdateCommand(command);
}

//---------------------------------------------------------------------------
int RemoteService::startUpdateCommand(UpdateCommand aCommand) {
    auto appInfo = m_Application->getAppInfo();

    // Сериализуем настройки прокси.
    auto *settings =
        SettingsService::instance(m_Application)->getAdapter<PPSDK::TerminalSettings>();

    QString commandParams = QString("--server \"%1\" --version \"%2\" --application %3 --conf %4 "
                                    "--id %5 --point %6 --accept-keys %7")
                                .arg(aCommand.configUrl.toString())
                                .arg(appInfo.version)
                                .arg(appInfo.appName)
                                .arg(appInfo.configuration)
                                .arg(aCommand.ID)
                                .arg(settings->getKeys()[0].ap)
                                .arg(settings->getKeys()[0].bankSerialNumber);

    auto proxy = settings->getConnection().proxy;
    if (proxy.type() != QNetworkProxy::NoProxy) {
        commandParams += QString(" --proxy %1:%2:%3:%4:%5")
                             .arg(proxy.hostName())
                             .arg(proxy.port())
                             .arg(proxy.user())
                             .arg(proxy.password())
                             .arg(proxy.type());
    }

    switch (aCommand.type) {
    case Configuration:
        commandParams += " --command config";
        if (!aCommand.parameters.isEmpty()) {
            commandParams += QString(" --md5 %1").arg(aCommand.parameters.first());
        }
        break;

    case Update:
        commandParams +=
            QString(" --command update --update-url \"%1\"").arg(aCommand.updateUrl.toString());
        if (!aCommand.parameters.isEmpty()) {
            commandParams += QString(" --components \"%1\"").arg(aCommand.parameters.join("#"));
        }
        break;

    case UserPack:
        commandParams += " --command userpack";
        if (!aCommand.parameters.isEmpty()) {
            commandParams += QString(" --md5 %1").arg(aCommand.parameters.first());
        }
        break;

    case AdUpdate:
        commandParams += " --command userpack --no-restart true --destination-subdir ad";
        if (!aCommand.parameters.isEmpty()) {
            commandParams += QString(" --md5 %1").arg(aCommand.parameters.first());
        }
        break;

    case FirmwareDownload:
        if (!aCommand.parameters.isEmpty()) {
            commandParams +=
                QString(" --command userpack --no-restart true --destination-subdir update/%1")
                    .arg(aCommand.parameters.at(1));
            commandParams += QString(" --md5 %1").arg(aCommand.parameters.at(0));
        }
        break;

    case CheckIntegrity:
        commandParams += " --command integrity";
        break;

    default:
        toLog(LogLevel::Error, QString("Unknown update command type: %1.").arg(aCommand.type));
        return 0;
    }

    aCommand.status = Executing;
    m_UpdateCommands.insert(aCommand.ID, aCommand);

    TerminalService::instance(m_Application)->getClient()->subscribeOnModuleClosed(this);
    TerminalService::instance(m_Application)
        ->getClient()
        ->startModule(CWatchService::Modules::Updater, commandParams);

    emit commandStatusChanged(aCommand.ID, aCommand.status, QVariantMap());

    saveCommandQueue();

    return aCommand.ID;
}

//---------------------------------------------------------------------------
void RemoteService::doScreenshotCommand() {
    while (!m_ScreenShotsCommands.isEmpty()) {
        int command = m_ScreenShotsCommands.takeFirst();

        QVariantList value;
        foreach (auto image, m_Application->getScreenshot()) {
            value.push_back(image);
        }

        QVariantMap parameters;
        parameters.insert(PPSDK::CMonitoringService::CommandParameters::Screenshots, value);

        emit commandStatusChanged(command, OK, parameters);
    }
}

//---------------------------------------------------------------------------
int RemoteService::registerScreenshotCommand() {
    QMutexLocker lock(&m_CommandMutex);

    int command = increaseLastCommandID();

    m_ScreenShotsCommands.push_back(command);
    QTimer::singleShot(100, this, SLOT(doScreenshotCommand()));

    return command;
}

//---------------------------------------------------------------------------
int RemoteService::registerGenerateKeyCommand(const QString &aLogin, const QString &aPassword) {
    QMutexLocker lock(&m_CommandMutex);

    if (m_GenerateKeyCommand == 0) {
        m_GenerateKeyCommand = increaseLastCommandID();

        m_GenerateKeyFuture = QtConcurrent::run(
            [this, aLogin, aPassword] { doGenerateKeyCommand(this, aLogin, aPassword); });

        return m_GenerateKeyCommand;
    }

    return 0;
}

//---------------------------------------------------------------------------
void RemoteService::doGenerateKeyCommand(RemoteService *aService,
                                         const QString &aLogin,
                                         const QString &aPassword) {
    auto *terminalSettings = dynamic_cast<PPSDK::TerminalSettings *>(
        aService->m_Application->getCore()->getSettingsService()->getAdapter(
            PPSDK::CAdapterNames::TerminalAdapter));
    if (!terminalSettings) {
        aService->toLog(LogLevel::Error, "GENKEY: Failed to get terminal settings.");
        aService->commandStatusChanged(aService->m_GenerateKeyCommand, Error, QVariantMap());
        return;
    }

    auto *cryptoService = aService->m_Application->getCore()->getCryptService();

    QString url(terminalSettings->getKeygenURL());
    QString SD; // NOLINT(readability-identifier-naming)
    QString AP; // NOLINT(readability-identifier-naming)
    QString OP; // NOLINT(readability-identifier-naming)
    int error = cryptoService->generateKey(
        CRemoteService::GeneratedKeyId, aLogin, aPassword, url, SD, AP, OP);
    if (error != 0) {
        aService->toLog(LogLevel::Error, QString("GENKEY: Error generate key: %1.").arg(error));
    } else {
        error = static_cast<int>(!cryptoService->saveKey());
        if (error != 0) {
            aService->toLog(LogLevel::Error, "GENKEY: Failed to save new key.");
        }
    }

    aService->commandStatusChanged(
        aService->m_GenerateKeyCommand, (error != 0) ? Error : OK, QVariantMap());

    aService->m_GenerateKeyCommand = (error != 0) ? 0 : aService->m_GenerateKeyCommand;

    if (error == 0) {
        aService->toLog(
            LogLevel::Normal,
            QString("GENKEY: New key %1 generated. Wait for send command status to server.")
                .arg(CRemoteService::GeneratedKeyId));

        aService->m_Database->setDeviceParam(
            SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal,
            CRemoteService::GenKeyCommandId,
            aService->m_GenerateKeyCommand);
        aService->m_Application->getCore()->getSettingsService()->saveConfiguration();
    }
}

//---------------------------------------------------------------------------
void RemoteService::commandStatusSent(int aCommandId, int aStatus) {
    if ((m_GenerateKeyCommand != 0) && aCommandId == m_GenerateKeyCommand && aStatus == OK) {
        m_GenerateKeyCommand = 0;

        if (m_Application->getCore()->getCryptService()->replaceKeys(CRemoteService::GeneratedKeyId,
                                                                     0)) {
            m_Application->getCore()->getSettingsService()->saveConfiguration();

            m_Database->setDeviceParam(SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal,
                                       CRemoteService::GenKeyCommandId,
                                       "");

            toLog(LogLevel::Normal, "GENKEY: Successful swap old and new crypto key.");
        } else {
            toLog(LogLevel::Error,
                  "GENKEY: Error swap old and new crypto key. Terminal will be restarted.");

            executeCommand(PPSDK::EEventType::Reboot);
        }
    }
}

//---------------------------------------------------------------------------
void RemoteService::updateContent() {
    foreach (auto client, m_MonitoringClients) {
        client->useCapability(PPSDK::IRemoteClient::UpdateContent);
    }
}

//---------------------------------------------------------------------------
void RemoteService::sendHeartbeat() {
    foreach (auto client, m_MonitoringClients) {
        client->useCapability(PPSDK::IRemoteClient::SendHeartbeat);
    }
}

//---------------------------------------------------------------------------
void RemoteService::onCheckQueuedRebootCommands() {
    QStringList ids =
        m_Database
            ->getDeviceParam(SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal,
                             CRemoteService::QueuedRebootCommands)
            .toString()
            .split(CRemoteService::QueuedRebootCommandDelimeter, Qt::SkipEmptyParts);

    foreach (const QString &id, ids) {
        emit commandStatusChanged(id.toInt(), OK, QVariantMap());
    }

    m_Database->setDeviceParam(SDK::PaymentProcessor::CDatabaseConstants::Devices::Terminal,
                               CRemoteService::QueuedRebootCommands,
                               QString());
}

//---------------------------------------------------------------------------
void RemoteService::onUpdateDirChanged() {
    m_CheckUpdateReportsTimer.stop();

    toLog(LogLevel::Normal, "Directory 'update' changed.");

    restartUpdateWatcher(dynamic_cast<QFileSystemWatcher *>(sender()));

    // Таймаут что бы этот обработчик, вызванный много раз в течении короткого времени, не запускал
    // долгую процедуру проверки файлов отчетов.
    m_CheckUpdateReportsTimer.start();
}

//---------------------------------------------------------------------------
QList<CommandReport> getReports(const QString &aReportsPath) {
    QList<CommandReport> result;

    foreach (auto reportFileName, QDir(aReportsPath, "*.rpt").entryList(QDir::Files)) {
        CommandReport r(aReportsPath + QDir::separator() + reportFileName);
        QSettings report(r.filePath, QSettings::IniFormat);

        QVariant idVar = report.value("id");
        r.id = idVar.isValid()
                   ? idVar.toInt()
                   : reportFileName
                         .mid(reportFileName.indexOf('_') + 1,
                              reportFileName.indexOf('.') - reportFileName.indexOf('_') - 1)
                         .toInt();

        r.status = static_cast<SDK::PaymentProcessor::IRemoteService::EStatus>(
            report.value("status").toInt());
        r.lastUpdate = QDateTime::fromString(report.value("last_update").toString(),
                                             CRemoteService::DateTimeFormat);
        r.description = report.value("status_desc");
        r.progress = report.value("progress");

        result << r;
    }

    return result;
}

//---------------------------------------------------------------------------
int RemoteService::checkUpdateReports() {
    int removedCount = 0;

    // Проверка репортов команд
    foreach (auto report, getReports(IApplication::getWorkingDirectory() + "/update/")) {
        QVariantMap parameters;

        if (report.description.isValid()) {
            parameters.insert(
                SDK::PaymentProcessor::CMonitoringService::CommandParameters::Description,
                report.description);
        }

        switch (report.status) {
        case OK: {
            parameters.insert(
                SDK::PaymentProcessor::CMonitoringService::CommandParameters::Progress, "100");

            // Start firmware upload?
            if (checkFirmwareUpload(report.id)) {
                report.remove();
                removedCount++;

                parameters.insert(
                    SDK::PaymentProcessor::CMonitoringService::CommandParameters::Progress, "50");
                emit commandStatusChanged(report.id, Executing, parameters);
                break;
            }
        }

        case Error: {
            updateCommandFinish(report.id, report.status, parameters);

            report.remove();
            removedCount++;

            break;
        }

        default: {
            if (report.progress.isValid()) {
                parameters.insert(
                    SDK::PaymentProcessor::CMonitoringService::CommandParameters::Progress,
                    report.progress);
            }

            if (report.isAlive()) {
                // Обновляем время изменения команды
                if (m_UpdateCommands.contains(report.id)) {
                    QMutexLocker lock(&m_CommandMutex);
                    m_UpdateCommands[report.id].lastUpdate = report.lastUpdate;
                    saveCommandQueue();
                }

                emit commandStatusChanged(report.id, Executing, parameters);
            } else {
                updateCommandFinish(report.id, Error, parameters);

                report.remove();
                removedCount++;
            }

            break;
        }
        }
    }

    return removedCount;
}

//---------------------------------------------------------------------------
void RemoteService::onCheckUpdateReports() {
    checkUpdateReports();
    checkCommandsLifetime();
}

//---------------------------------------------------------------------------
int RemoteService::checkCommandsLifetime() {
    int removed = 0;

    foreach (auto cmd, m_UpdateCommands.values()) {
        if (cmd.lastUpdate.addSecs(CRemoteService::MaxUpdateCommandLifetime) <
            QDateTime::currentDateTime()) {
            toLog(LogLevel::Error,
                  QString("Command %1 lifetime has ended. (%2)")
                      .arg(cmd.ID)
                      .arg(cmd.configUrl.toString()));

            QMutexLocker lock(&m_CommandMutex);
            m_UpdateCommands.remove(cmd.ID);
            saveCommandQueue();
            removed++;

            emit commandStatusChanged(cmd.ID, Error, QVariantMap());
        }
    }

    return removed;
}

//---------------------------------------------------------------------------
bool RemoteService::checkFirmwareUpload(int aCommandID) {
    if (!m_UpdateCommands.contains(aCommandID)) {
        return false;
    }

    auto command = m_UpdateCommands.value(aCommandID);

    if (command.type != IRemoteService::FirmwareDownload) {
        return false;
    }

    command.type = IRemoteService::FirmwareUpload;
    command.status = IRemoteService::Executing;
    m_UpdateCommands.insert(aCommandID, command);
    saveCommandQueue();

    toLog(LogLevel::Normal,
          QString("Complete download firmware. Restart command %1 for UPLOAD.").arg(aCommandID));

    EventService::instance(m_Application)
        ->sendEvent(PPSDK::Event(
            PPSDK::EEventType::Restart, "updater", QString("-start_scenario=FirmwareUpload")));

    return true;
}

//---------------------------------------------------------------------------
bool RemoteService::restoreConfiguration() {
    bool result = false;

    foreach (PPSDK::IRemoteClient *client, m_MonitoringClients) {
        if ((client->getCapabilities() & PPSDK::IRemoteClient::RestoreConfiguration) != 0U) {
            result = client->useCapability(PPSDK::IRemoteClient::RestoreConfiguration);
        }
    }

    return result;
}

//---------------------------------------------------------------------------
void RemoteService::saveCommandQueue() {
    m_Settings.clear();

    foreach (auto command, m_UpdateCommands) {
        m_Settings.beginGroup(QString("cmd_%1").arg(command.ID));

        m_Settings.setValue("id", command.ID);
        m_Settings.setValue("configUrl", command.configUrl.toString());
        m_Settings.setValue("updateUrl", command.updateUrl.toString());
        m_Settings.setValue("type", command.type);
        m_Settings.setValue("parameters", command.parameters.join("#"));
        m_Settings.setValue("status", command.status);
        m_Settings.setValue("lastUpdate",
                            command.lastUpdate.toString(CRemoteService::DateTimeFormat));

        m_Settings.endGroup();
    }

    QStringList screenShotsCommands;
    foreach (int cmd, m_ScreenShotsCommands) {
        screenShotsCommands << QString::number(cmd);
    }

    m_Settings.setValue("common/screenshot", screenShotsCommands.join(";"));

    m_Settings.sync();
}

//---------------------------------------------------------------------------
void RemoteService::restoreCommandQueue() {
    m_Settings.sync();

    foreach (auto group, m_Settings.childGroups()) {
        UpdateCommand cmd;

        m_Settings.beginGroup(group);

        cmd.ID = m_Settings.value("id").toInt();
        cmd.configUrl = m_Settings.value("configUrl").toUrl();
        cmd.updateUrl = m_Settings.value("updateUrl").toUrl();
        cmd.type = static_cast<EUpdateType>(m_Settings.value("type").toInt());
        cmd.status = static_cast<EStatus>(m_Settings.value("status").toInt());
        cmd.parameters = m_Settings.value("parameters").toString().split("#");
        cmd.lastUpdate = QDateTime::fromString(m_Settings.value("lastUpdate").toString(),
                                               CRemoteService::DateTimeFormat);

        // Если метка времени до сих пор не использовалась, заполняем её текущим временем.
        if (cmd.lastUpdate.isNull() || !cmd.lastUpdate.isValid()) {
            cmd.lastUpdate = QDateTime::currentDateTime();
            m_Settings.setValue("lastUpdate",
                                cmd.lastUpdate.toString(CRemoteService::DateTimeFormat));
        }

        if (cmd.ID != 0) {
            m_UpdateCommands.insert(cmd.ID, cmd);
        }

        m_Settings.endGroup();
    }

    foreach (auto cmd, m_Settings.value("common/screenshot").toString().split(";")) {
        m_ScreenShotsCommands << cmd.toInt();
    }
}

//---------------------------------------------------------------------------
RemoteService::UpdateCommand RemoteService::findUpdateCommand(EUpdateType aType) {
    foreach (auto command, m_UpdateCommands.values()) {
        if (command.type == aType) {
            return command;
        }
    }

    return {};
}

//---------------------------------------------------------------------------
void RemoteService::restartUpdateWatcher(QFileSystemWatcher *aWatcher) {
    if (!aWatcher) {
        aWatcher = new QFileSystemWatcher(this);
        connect(
            aWatcher, SIGNAL(directoryChanged(const QString &)), this, SLOT(onUpdateDirChanged()));
        connect(aWatcher, SIGNAL(fileChanged(const QString &)), this, SLOT(onUpdateDirChanged()));
        aWatcher->addPath(IApplication::getWorkingDirectory() + "/update");
    }

    QStringList files;
    foreach (
        auto name,
        QDir(IApplication::getWorkingDirectory() + "/update", "*.rpt").entryInfoList(QDir::Files)) {
        files << name.filePath();
    }

    if (!files.isEmpty()) {
        aWatcher->addPaths(files);
    }
}

//---------------------------------------------------------------------------
void RemoteService::onModuleClosed(const QString &aModuleName) {
    if (aModuleName == CWatchService::Modules::Updater && checkUpdateReports() == 0) {
        // Updater закрылся, но команда осталась в очереди - удаляем эту команду.
        foreach (auto cmd, m_UpdateCommands.values()) {
            if (cmd.isExternal() && cmd.status == Executing) {
                toLog(LogLevel::Warning,
                      QString(
                          "Remove command %1 (type=%2) because updater module was closed. URL:%3.")
                          .arg(cmd.ID)
                          .arg(cmd.type)
                          .arg(cmd.configUrl.toString()));

                updateCommandFinish(cmd.ID, Error);

                break;
            }
        }
    }
}

//---------------------------------------------------------------------------
void RemoteService::onDeviceStatusChanged(const QString &aConfigName) {
    if (aConfigName.contains(SDK::Driver::CComponents::Watchdog) ||
        aConfigName.contains("Terminal", Qt::CaseInsensitive)) {
        foreach (auto client, m_MonitoringClients) {
            client->useCapability(PPSDK::IRemoteClient::ReportStatus);
        }
    }
}

//---------------------------------------------------------------------------
void RemoteService::timerEvent(QTimerEvent *aEvent) {
    Q_UNUSED(aEvent)

    onCheckUpdateReports();

    startNextUpdateCommand();
}

//---------------------------------------------------------------------------
void RemoteService::updateCommandFinish(int aCmdID, EStatus aStatus, QVariantMap aParameters) {
    emit commandStatusChanged(aCmdID, aStatus, aParameters);

    if (m_UpdateCommands.contains(aCmdID)) {
        QMutexLocker lock(&m_CommandMutex);

        m_UpdateCommands.remove(aCmdID);
        saveCommandQueue();
    }

    startNextUpdateCommand();
}

//---------------------------------------------------------------------------
void RemoteService::startNextUpdateCommand() {
    UpdateCommand nextCommand;

    QMutexLocker lock(&m_CommandMutex);

    foreach (int id, m_UpdateCommands.keys()) {
        if (m_UpdateCommands[id].status == IRemoteService::Waiting) {
            nextCommand = m_UpdateCommands[id];
            break;
        }
    }

    // запускаем следующую команду, если это возможно
    if (nextCommand.isValid()) {
        m_UpdateCommands.remove(nextCommand.ID);

        bool haveExecuted = false;

        foreach (auto cmd, m_UpdateCommands.values()) {
            if (cmd.status != IRemoteService::Waiting) {
                haveExecuted = true;
                break;
            }
        }

        if (!haveExecuted && m_GenerateKeyCommand == 0 && m_QueuedRebootCommands.isEmpty()) {
            startUpdateCommand(nextCommand);
        } else {
            m_UpdateCommands.insert(nextCommand.ID, nextCommand);
        }
    }
}

//---------------------------------------------------------------------------
RemoteService::UpdateCommand::UpdateCommand()
    : ID(-1), status(IRemoteService::Waiting), type(Configuration) {
    lastUpdate = QDateTime::currentDateTime();
}

//---------------------------------------------------------------------------
bool RemoteService::UpdateCommand::isValid() const {
    return ID >= 0;
}

//---------------------------------------------------------------------------
bool RemoteService::UpdateCommand::isExternal() const {
    return type != FirmwareUpload;
}

//---------------------------------------------------------------------------
