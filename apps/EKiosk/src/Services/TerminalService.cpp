/* @file Сценарий управления терминалом. */

#include "TerminalService.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QSet>
#include <QtCore/QTimer>

#include <Common/BasicApplication.h>
#include <Common/ExceptionFilter.h>
#include <Common/Version.h>

#include <SDK/Drivers/CashAcceptor/CashAcceptorStatus.h>
#include <SDK/Drivers/DeviceTypes.h>
#include <SDK/Drivers/WarningLevel.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <Packer/Packer.h>
#include <SysUtils/ISysUtils.h>
#include <WatchServiceClient/Constants.h>
#include <algorithm>

#include "DatabaseUtils/IHardwareDatabaseUtils.h"
#include "NetworkTaskManager/MemoryDataStream.h"
#include "NetworkTaskManager/NetworkTask.h"
#include "NetworkTaskManager/NetworkTaskManager.h"
#include "Services/CryptService.h"
#include "Services/DatabaseService.h"
#include "Services/EventService.h"
#include "Services/GUIService.h"
#include "Services/RemoteService.h"
#include "Services/ServiceNames.h"
#include "Services/SettingsService.h"
#include "System/IApplication.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
TerminalService *TerminalService::instance(IApplication *aApplication) {
    return dynamic_cast<TerminalService *>(
        aApplication->getCore()->getService(CServices::TerminalService));
}

//---------------------------------------------------------------------------
TerminalService::TerminalService(IApplication *aApplication)
    : m_DbUtils(nullptr), m_EventService(m_Application->getCore()->getEventService()),
      m_Application(aApplication),
      m_Client(createWatchServiceClient(CWatchService::Modules::PaymentProcessor,
                                        IWatchServiceClient::MainThread)) {
    setLog(m_Application->getLog());

    m_EventService->subscribe(this, SLOT(onEvent(const SDK::PaymentProcessor::Event &)));
}

//---------------------------------------------------------------------------
TerminalService::~TerminalService() = default;

//---------------------------------------------------------------------------
bool TerminalService::initialize() {
    DatabaseService *dbService = DatabaseService::instance(m_Application);

    m_DbUtils = dbService->getDatabaseUtils<IHardwareDatabaseUtils>();

    // Добавляем устройство "терминал" в БД.
    if (!m_DbUtils->hasDevice(PPSDK::CDatabaseConstants::Devices::Terminal)) {
        if (!m_DbUtils->addDevice(PPSDK::CDatabaseConstants::Devices::Terminal)) {
            toLog(LogLevel::Error, "Failed to add a record to database.");
            return false;
        }
    }

    m_Settings = SettingsService::instance(m_Application)
                     ->getAdapter<PPSDK::TerminalSettings>()
                     ->getCommonSettings();

    PPSDK::IDeviceService *deviceService = m_Application->getCore()->getDeviceService();
    connect(deviceService,
            SIGNAL(deviceStatusChanged(
                const QString &, SDK::Driver::EWarningLevel::Enum, const QString &, int)),
            SLOT(onDeviceStatusChanged(
                const QString &, SDK::Driver::EWarningLevel::Enum, const QString &, int)));

    connect(deviceService, SIGNAL(configurationUpdated()), SLOT(onHardwareConfigUpdated()));

    // Проверяем наличие ключей.
    auto key = CryptService::instance(m_Application)->getKey(0);

    if (!key.isValid) {
        setTerminalError(PPSDK::ETerminalError::KeyError, true);
    } else {
        checkConfigsIntegrity();
    }

    // Обновляем количество запусков ПО.
    int restartCount = getRestartCount();

    restartCount++;

    setRestartCount(restartCount);

    // Проверяем состояние блокировки терминала
    if (m_DbUtils
            ->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                             PPSDK::CDatabaseConstants::Parameters::DisabledParam)
            .isNull()) {
        writeLockStatus(false);
    }

    m_DbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                              PPSDK::CDatabaseConstants::Parameters::Configuration,
                              m_Application->getSettings().value("common/configuration"));

    m_DbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                              PPSDK::CDatabaseConstants::Parameters::OperationSystem,
                              ISysUtils::getOSVersionInfo());

    // Проверяем параметры командной строки.
    bool standaloneMode = m_Application->getSettings().value("common/standalone").toBool();

    if (!m_Client) {
        if (!standaloneMode) {
            toLog(LogLevel::Error, "Failed to create WatchService client.");
            return false;
        }
        toLog(LogLevel::Warning, "WatchService client not available, running in standalone mode.");
        return true;
    }

    if (m_Client->start() || standaloneMode) {
        return true;
    }

    toLog(LogLevel::Error, "Watchdog service module is not available.");
    return false;
}

//------------------------------------------------------------------------------
void TerminalService::finishInitialize() {
    auto *guiService = GUIService::instance(m_Application);

    if (guiService) {
        QStringList screenList;
        QRect resolution;

        for (int screenIndex = 0;; ++screenIndex) {
            resolution = guiService->getScreenSize(screenIndex);

            if (resolution.isEmpty()) {
                break;
            }

            screenList << QString(R"({"width":%1,"height":%2})")
                              .arg(resolution.width())
                              .arg(resolution.height());
        }

        m_DbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                                  QString(PPSDK::CDatabaseConstants::Parameters::DisplayResolution),
                                  QString("[%1]").arg(screenList.join(",")));
    }

    // Получаем информацию о дефолтном ключе
    auto *terminalSettings =
        SettingsService::instance(m_Application)->getAdapter<PPSDK::TerminalSettings>();
    PPSDK::SKeySettings key = terminalSettings->getKeys().value(0);

    // Устанавливаем User-Agent (Имя_ПО + Версия_ПО + код_дилера + код_точки + код_оператора)
    QString userAgent = Humo::Application + " " + Humo::getVersion();
    if (key.isValid) {
        userAgent.append(" SD:" + key.sd + " AP:" + key.ap + " OP:" + key.op);
    }

    PPSDK::INetworkService *networkService = m_Application->getCore()->getNetworkService();
    networkService->setUserAgent(userAgent);
}

//---------------------------------------------------------------------------
bool TerminalService::canShutdown() {
    return true;
}

//---------------------------------------------------------------------------
bool TerminalService::shutdown() {
    // Показываем экран блокировки WatchService.
    m_Client->resetState();
    m_Client->stop();

    return true;
}

//---------------------------------------------------------------------------
QString TerminalService::getName() const {
    return CServices::TerminalService;
}

//---------------------------------------------------------------------------
const QSet<QString> &TerminalService::getRequiredServices() const {
    static QSet<QString> required = QSet<QString>()
                                    << CServices::DatabaseService << CServices::EventService
                                    << CServices::SettingsService << CServices::DeviceService
                                    << CServices::CryptService;

    return required;
}

//---------------------------------------------------------------------------
QVariantMap TerminalService::getParameters() const {
    QVariantMap parameters;

    parameters[PPSDK::CServiceParameters::Terminal::RestartCount] = getRestartCount();

    return parameters;
}

//---------------------------------------------------------------------------
bool TerminalService::isLocked() const {
    return isDisabled();
}

//---------------------------------------------------------------------------
void TerminalService::setLock(bool aIsLocked) {
    // Отключаем интерфейс.
    auto *guiService = GUIService::instance(m_Application);

    if (guiService) {
        guiService->disable(aIsLocked);
    }

    writeLockStatus(aIsLocked);
}

//---------------------------------------------------------------------------
void TerminalService::resetParameters(const QSet<QString> &aParameters) {
    if (aParameters.contains(PPSDK::CServiceParameters::Terminal::RestartCount)) {
        m_DbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                                  PPSDK::CDatabaseConstants::Parameters::LaunchCount,
                                  0);
    }
}

//---------------------------------------------------------------------------
void TerminalService::writeLockStatus(bool aIsLocked) {
    // Если разблокируем, то все критические ошибки превращаем в некритические (кроме ошибки БД)
    if (!aIsLocked) {
        foreach (auto key, m_TerminalStatusHash.keys()) {
            if (key != CServices::DatabaseService &&
                m_TerminalStatusHash[key].getType() >= PPSDK::EEventType::Warning) {
                m_TerminalStatusHash.remove(key);
            } else if (m_TerminalStatusHash[key].getData().toString().contains("#alarm")) {
                PPSDK::Event e = m_TerminalStatusHash[key];

                m_TerminalStatusHash[key] = PPSDK::Event(
                    e.getType(), e.getSender(), e.getData().toString().remove("#alarm"));
            }
        }

        // Сбрасываем "плохие" статусы при разблокировке
        TStatusCodes statuses = QSet<int>(
            m_DeviceErrorFlags.values(PPSDK::CDatabaseConstants::Devices::Terminal).begin(),
            m_DeviceErrorFlags.values(PPSDK::CDatabaseConstants::Devices::Terminal).end());

        foreach (auto status, statuses) {
            if (TerminalStatusCode::Specification[status].warningLevel >=
                SDK::Driver::EWarningLevel::Warning) {
                m_DeviceErrorFlags.remove(PPSDK::CDatabaseConstants::Devices::Terminal, status);
            }
        }
    }

    updateTerminalStatus();

    // Проверка на "Уже записали статус в БД?", иначе при блокировке по ошибке БД мы зацикливаемся
    if (m_Locked.is_initialized() && m_Locked.get() == aIsLocked) {
        return;
    }

    m_Locked = aIsLocked;

    m_DbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                              PPSDK::CDatabaseConstants::Parameters::DisabledParam,
                              m_Locked.get());
}

//---------------------------------------------------------------------------
bool TerminalService::isDisabled() const {
    if (m_Locked.is_initialized()) {
        return m_Locked.get();
    }

    m_Locked = m_DbUtils
                   ->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                                    PPSDK::CDatabaseConstants::Parameters::DisabledParam)
                   .toBool();

    return m_Locked.get();
}

//---------------------------------------------------------------------------
// TODO: перейти от ивентов к методам интерфейса ITerminalService.
void TerminalService::onEvent(const SDK::PaymentProcessor::Event &aEvent) {
    auto eventType = static_cast<PPSDK::EEventType::Enum>(aEvent.getType());

    switch (eventType) {
    // Выключить терминал.
    case PPSDK::EEventType::TerminalLock: {
        setLock(true);
        break;
    }

    // Включить терминал.
    case PPSDK::EEventType::TerminalUnlock: {
        setLock(false);
        break;
    }

    case PPSDK::EEventType::RestoreConfiguration: {
        if (!RemoteService::instance(m_Application)->restoreConfiguration()) {
            toLog(LogLevel::Error, "Failed to execute restore configuration command.");
        }

        break;
    }

    case PPSDK::EEventType::OK:
    case PPSDK::EEventType::Warning:
    case PPSDK::EEventType::Critical: {
        m_TerminalStatusHash[aEvent.getSender()] = aEvent;

        if (aEvent.getSender() == CServices::DatabaseService) {
            setTerminalError(PPSDK::ETerminalError::DatabaseError,
                             aEvent.getType() == PPSDK::EEventType::Critical);
        } else if (aEvent.getSender() == "AccountBalance") {
            setTerminalError(PPSDK::ETerminalError::AccountBalanceError,
                             eventType != PPSDK::EEventType::OK);
        } else {
            updateTerminalStatus();
        }

        break;
    }

    // Обработка остальных типов событий - игнорируем
    default:
        break;
    }
}

//---------------------------------------------------------------------------
QPair<SDK::Driver::EWarningLevel::Enum, QString> TerminalService::getTerminalStatus() const {
    QStringList resultMessage;
    SDK::Driver::EWarningLevel::Enum resultLevel = SDK::Driver::EWarningLevel::OK;

    auto convertStatus =
        [](PPSDK::EEventType::Enum aEventType) -> SDK::Driver::EWarningLevel::Enum {
        switch (aEventType) {
        case PPSDK::EEventType::Warning:
            return SDK::Driver::EWarningLevel::Warning;
        case PPSDK::EEventType::Critical:
            return SDK::Driver::EWarningLevel::Error;
        default:
            return SDK::Driver::EWarningLevel::OK;
        }
    };

    QMapIterator<QString, SDK::PaymentProcessor::Event> i(m_TerminalStatusHash);
    while (i.hasNext()) {
        i.next();

        auto status = convertStatus(static_cast<PPSDK::EEventType::Enum>(i.value().getType()));
        resultLevel = std::max(status, resultLevel);

        QString data = i.value().getData().toString();

        if (status != SDK::Driver::EWarningLevel::OK || (!data.isEmpty() && data != "OK")) {
            resultMessage
                << QString("%1: %2").arg(i.value().getSender()).arg(i.value().getData().toString());
        }
    }

    TStatusCodes statuses =
        QSet<int>(m_DeviceErrorFlags.values(PPSDK::CDatabaseConstants::Devices::Terminal).begin(),
                  m_DeviceErrorFlags.values(PPSDK::CDatabaseConstants::Devices::Terminal).end());

    if (statuses.isEmpty()) {
        statuses << DeviceStatusCode::OK::OK;
    }

    foreach (int status, statuses) {
        resultMessage << TerminalStatusCode::Specification[status].translation;

        resultLevel = std::max(resultLevel, TerminalStatusCode::Specification[status].warningLevel);
    }

    if (isLocked()) {
        resultMessage << "Locked";
        resultLevel = SDK::Driver::EWarningLevel::Error;
    }

    if (resultMessage.isEmpty()) {
        resultMessage << "OK";
    }

    return {resultLevel, resultMessage.join("\n")};
}

//---------------------------------------------------------------------------
void TerminalService::onDeviceStatusChanged(const QString &aConfigName,
                                            SDK::Driver::EWarningLevel::Enum aLevel,
                                            const QString &aDescription,
                                            int aStatus) {
    Q_UNUSED(aDescription);

    namespace DeviceType = SDK::Driver::CComponents;

    QString deviceType = aConfigName.section('.', 2, 2);

    if (getAcceptorTypes().contains(deviceType) || DeviceType::isPrinter(deviceType) ||
#ifdef TC_USE_TOKEN
        deviceType == DeviceType::Token ||
#endif
        deviceType == DeviceType::CardReader) {
        if (aLevel == SDK::Driver::EWarningLevel::Error ||
            SDK::Driver::EStatus::Interface == aStatus) {
            // Device has entered error state
            m_DeviceErrorFlags.insert(aConfigName, aStatus);
        } else {
            // Device has exited error state
            m_DeviceErrorFlags.remove(aConfigName);
        }
    }

    bool autoencashment = false;

    // Авто инкассация.
    if (deviceType == DeviceType::BillAcceptor &&
        aStatus == SDK::Driver::ECashAcceptorStatus::StackerOpen && m_Settings.autoEncashment) {
        // Запускаем авто инкассацию только тогда, когда нет ошибок у валидатора.
        QList<int> validatorErrorFlags = m_DeviceErrorFlags.values(
            m_DeviceErrorFlags.key(SDK::Driver::ECashAcceptorStatus::StackerOpen));

        validatorErrorFlags.removeAll(SDK::Driver::ECashAcceptorStatus::StackerOpen);

        autoencashment = validatorErrorFlags.isEmpty();
    }

    if (autoencashment) {
        EventService::instance(m_Application)
            ->sendEvent(PPSDK::EEventType::Autoencashment, QVariantMap());
    } else {
        updateGUI();
    }
}

//---------------------------------------------------------------------------
void TerminalService::setTerminalError(PPSDK::ETerminalError::Enum aErrorType, bool aError) {
    bool changed = false;

    if (aError) {
        if (!m_DeviceErrorFlags.contains(PPSDK::CDatabaseConstants::Devices::Terminal,
                                         aErrorType)) {
            m_DeviceErrorFlags.insert(PPSDK::CDatabaseConstants::Devices::Terminal, aErrorType);

            changed = true;
        }
    } else if (m_DeviceErrorFlags.contains(PPSDK::CDatabaseConstants::Devices::Terminal,
                                           aErrorType)) {
        m_DeviceErrorFlags.remove(PPSDK::CDatabaseConstants::Devices::Terminal, aErrorType);

        changed = true;
    }

    if (changed) {
        updateGUI();

        updateTerminalStatus();
    }
}

//---------------------------------------------------------------------------
void TerminalService::updateGUI() {
    auto *guiService = GUIService::instance(m_Application);

    if (guiService) {
        guiService->disable(!getFaultyDevices(true).isEmpty() || isLocked());
    }
}

//---------------------------------------------------------------------------
bool TerminalService::isTerminalError(PPSDK::ETerminalError::Enum aErrorType) const {
    return m_DeviceErrorFlags.values(PPSDK::CDatabaseConstants::Devices::Terminal)
        .contains(aErrorType);
}

//---------------------------------------------------------------------------
void TerminalService::updateTerminalStatus() {
    auto warningLevel2LogLevel = [](SDK::Driver::EWarningLevel::Enum aEventType) -> LogLevel::Enum {
        switch (aEventType) {
        case SDK::Driver::EWarningLevel::Warning:
            return LogLevel::Warning;
        case SDK::Driver::EWarningLevel::Error:
            return LogLevel::Error;
        default:
            return LogLevel::Normal;
        }
    };

    auto status = getTerminalStatus();

    if (m_TerminalStatusCache != status) {
        toLog(warningLevel2LogLevel(status.first),
              QString("Terminal status: %1.")
                  .arg(status.second.replace("\r", "").replace("\n", ";")));

        m_DbUtils->addDeviceStatus(
            PPSDK::CDatabaseConstants::Devices::Terminal, status.first, status.second);
    }
}

//---------------------------------------------------------------------------
void TerminalService::checkConfigsIntegrity() {
    // Проверяем корректность загруженных настроек.
    bool valid = true;

    foreach (const PPSDK::ISettingsAdapter *settings,
             SettingsService::instance(m_Application)->enumerateAdapters()) {
        valid = valid && settings->isValid();
    }

    namespace DbConstants = PPSDK::CDatabaseConstants;

    if (!valid) {
        setTerminalError(PPSDK::ETerminalError::ConfigError, true);

        // Проверяем время с прошедшего обновления.
        QDateTime lastTryTime = m_DbUtils
                                    ->getDeviceParam(DbConstants::Devices::Terminal,
                                                     DbConstants::Parameters::LastUpdateTime)
                                    .toDateTime();
        QDateTime now = QDateTime::currentDateTime();

        if (!lastTryTime.isValid() || lastTryTime.addSecs(60LL * ConfigRestoreInterval) <= now) {
            EventService::instance(m_Application)
                ->sendEvent(PPSDK::EEventType::RestoreConfiguration, QVariant());

            m_DbUtils->setDeviceParam(
                DbConstants::Devices::Terminal, DbConstants::Parameters::LastUpdateTime, now);
        } else {
            qint64 retryTimeoutSecs = now.secsTo(lastTryTime.addSecs(60LL * ConfigRestoreInterval));
            int retryTimeout = static_cast<int>(retryTimeoutSecs);

            toLog(LogLevel::Warning,
                  QString("Configuration restore failed: maximum attemps reached. Will try again "
                          "in %1 sec.")
                      .arg(retryTimeout));

            // Запускаем таймер на повторение операции.
            QTimer::singleShot(retryTimeout * 1000, this, SLOT(checkConfigsIntegrity()));
        }
    }
}

//---------------------------------------------------------------------------
void TerminalService::sendFeedback(const QString &aSenderSubsystem, const QString &aMessage) {
    // Отбрасываем дубли сообщений
    static QSet<QString> sentMessages;

    QString url = dynamic_cast<PPSDK::TerminalSettings *>(
                      m_Application->getCore()->getSettingsService()->getAdapter(
                          PPSDK::CAdapterNames::TerminalAdapter))
                      ->getFeedbackURL();

    if (!url.isEmpty() && !sentMessages.contains(aMessage)) {
        QByteArray sendBody;

        sendBody += "complain=1&offer=1&";
        sendBody += "PMVer=" + Humo::getVersion().toUtf8().toPercentEncoding() + "&";

        auto key = CryptService::instance(m_Application)->getKey(0);
        sendBody += QString("SD=%1&AP=%2&OP=%3&").arg(key.sd).arg(key.ap).arg(key.op).toUtf8();

        sendBody += "Display=AxB&";
        sendBody += "WinVer=" + ISysUtils::getOSVersionInfo().toUtf8().toPercentEncoding() + "&";

        sendBody += "KKM=" + QString("%1 (%2) BankKey=%3")
                                 .arg("")
                                 .arg("")
                                 .arg(key.bankSerialNumber)
                                 .toUtf8()
                                 .toPercentEncoding();
        sendBody += "&";

        sendBody +=
            "subject=" + QString("TC3:%1").arg(aSenderSubsystem).toUtf8().toPercentEncoding() + "&";
        sendBody += "email=" + QString("help@humo.tj").toUtf8().toPercentEncoding() + "&";
        sendBody += "message=" + aMessage.toUtf8().toPercentEncoding() + "&";

        if (qint64 paymentId = m_Application->getCore()->getPaymentService()->getActivePayment()) {
            sendBody += "pluslog=1&";
            QByteArray zipArray = "[{\"fields\":{";

            auto fields =
                m_Application->getCore()->getPaymentService()->getPaymentFields(paymentId);

            foreach (auto f, fields) {
                zipArray += QString(R"("%1":"%2",)").arg(f.name).arg(f.value.toString()).toUtf8();
            }

            zipArray += "}}";

            double changeAmount = m_Application->getCore()->getPaymentService()->getChangeAmount();

            if (!qFuzzyIsNull(changeAmount) && changeAmount > 0.) {
                zipArray +=
                    QString(R"(,{"CHANGE_AMOUNT":"%1"})").arg(changeAmount, 0, 'f', 2).toUtf8();
            }

            zipArray += "]";

            QByteArray gz;
            if (Packer::gzipCompress(zipArray, QString("payment_%1.json").arg(paymentId), gz)) {
                sendBody += "pluslog_val=";
                sendBody += gz.toBase64().toPercentEncoding() + "&";
            }
        }

        auto *task = new NetworkTask();
        task->setUrl(url);
        task->setType(NetworkTask::Type::Post);
        task->setTimeout(60 * 1000);
        task->setDataStream(new MemoryDataStream());
        task->getRequestHeader().insert("Content-Type", "application/x-www-form-urlencoded");
        task->getDataStream()->write(sendBody);

        m_Application->getCore()->getNetworkService()->getNetworkTaskManager()->addTask(task);
        sentMessages.insert(aMessage);
    }
}

//---------------------------------------------------------------------------
void TerminalService::needUpdateConfigs() {
    // Переименовываем config.xml, тем самым конфиги будут скачаны заново при следующем запуске.
    QString backupExt = QDateTime::currentDateTime().toString(".yyyy-MM-dd_hh-mm-ss") + "_backup";
    QString config = QString("%1%2%3")
                         .arg(m_Application->getUserDataPath())
                         .arg(QDir::separator())
                         .arg("config.xml");

    if (!QFile::rename(config, config + backupExt)) {
        toLog(LogLevel::Error, "Failed to backup config.xml.");
    }
}

//---------------------------------------------------------------------------
QStringList TerminalService::getAcceptorTypes() const {
    return QStringList() << SDK::Driver::CComponents::BillAcceptor
                         << SDK::Driver::CComponents::CoinAcceptor;
}

//---------------------------------------------------------------------------
QStringList TerminalService::getDeviceNames() const {
    return QStringList(m_Application->getCore()->getDeviceService()->getConfigurations())
           << PPSDK::CDatabaseConstants::Devices::Terminal;
}

//---------------------------------------------------------------------------
void TerminalService::onHardwareConfigUpdated() {
    // Согласовываем список устройств в состоянии ошибки с текущим списком устройств.
    QStringList configNames = getDeviceNames();

    foreach (auto configName, m_DeviceErrorFlags.keys()) {
        if (!configNames.contains(configName)) {
            m_DeviceErrorFlags.remove(configName);
        }
    }
}

//---------------------------------------------------------------------------
IWatchServiceClient *TerminalService::getClient() {
    return m_Client.data();
}

//---------------------------------------------------------------------------
QMultiMap<QString, int> TerminalService::getFaultyDevices(bool aActual) const {
    namespace DeviceType = SDK::Driver::CComponents;

    auto result = m_DeviceErrorFlags;

    if (aActual) {
        QStringList cashAcceptors = getAcceptorTypes();
        QStringList faultyCashDeviceNames;
        QStringList cashDeviceNames;

        foreach (auto name, result.keys()) {
            if (cashAcceptors.contains(name.section('.', 2, 2))) {
                faultyCashDeviceNames << name;
            }
        }

        foreach (auto name, getDeviceNames()) {
            if (cashAcceptors.contains(name.section('.', 2, 2))) {
                cashDeviceNames << name;
            }
        }

        bool hasValidatorError =
            QSet<QString>(cashDeviceNames.begin(), cashDeviceNames.end()) ==
            QSet<QString>(faultyCashDeviceNames.begin(), faultyCashDeviceNames.end());

        foreach (const QString &device, result.keys()) {
            QString deviceType = device.section('.', 2, 2);

            if ((cashAcceptors.contains(deviceType) &&
                 (!m_Settings.blockOn(PPSDK::SCommonSettings::ValidatorError) ||
                  !hasValidatorError)) ||
                (DeviceType::isPrinter(deviceType) &&
                 !m_Settings.blockOn(PPSDK::SCommonSettings::PrinterError)) ||
#ifndef TC_USE_TOKEN
                (deviceType == DeviceType::Token) ||
#endif
                (deviceType == DeviceType::CardReader &&
                 !m_Settings.blockOn(PPSDK::SCommonSettings::CardReaderError))) {
                result.remove(device);
            }
        }
    }

    return result;
}

//---------------------------------------------------------------------------
void TerminalService::setRestartCount(int aCount) {
    QDate lastStartDate = m_DbUtils
                              ->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                                               PPSDK::CDatabaseConstants::Parameters::LastStartDate)
                              .toDate();
    QDate currentDate = QDate::currentDate();

    if (lastStartDate != currentDate) {
        m_DbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                                  PPSDK::CDatabaseConstants::Parameters::LastStartDate,
                                  currentDate);
        aCount = 0;
    }

    m_DbUtils->setDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                              PPSDK::CDatabaseConstants::Parameters::LaunchCount,
                              aCount);
}

//---------------------------------------------------------------------------
int TerminalService::getRestartCount() const {
    int count = m_DbUtils
                    ->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                                     PPSDK::CDatabaseConstants::Parameters::LaunchCount)
                    .toInt();
    QDate lastStartDate = m_DbUtils
                              ->getDeviceParam(PPSDK::CDatabaseConstants::Devices::Terminal,
                                               PPSDK::CDatabaseConstants::Parameters::LastStartDate)
                              .toDate();

    if (lastStartDate != QDate::currentDate()) {
        count = 0;
    }

    return count;
}

//---------------------------------------------------------------------------
