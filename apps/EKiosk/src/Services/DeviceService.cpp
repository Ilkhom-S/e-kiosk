/* @file Реализация сервиса для работы с устройствами. */

#include "DeviceService.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFileInfo>

#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/DeviceStatus.h>
#include <SDK/Drivers/FR/FiscalFields.h>
#include <SDK/Drivers/HardwareConstants.h>
#include <SDK/Drivers/InteractionTypes.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <algorithm>
#include <utility>

#include "DatabaseUtils/IHardwareDatabaseUtils.h"
#include "DeviceManager/DeviceManager.h"
#include "Services/DatabaseService.h"
#include "Services/PluginService.h"
#include "Services/ServiceNames.h"
#include "Services/SettingsService.h"
#include "System/IApplication.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
DeviceService *DeviceService::instance(IApplication *aApplication) {
    return dynamic_cast<DeviceService *>(
        aApplication->getCore()->getService(CServices::DeviceService));
}

//------------------------------------------------------------------------------
DeviceService::Status::Status()
    : m_Level(SDK::Driver::EWarningLevel::Error), m_Description(tr("#status_undefined")),
      m_Status(0) {}

//------------------------------------------------------------------------------
DeviceService::Status::Status(SDK::Driver::EWarningLevel::Enum aLevel,
                              QString aDescription,
                              int aStatus)
    : m_Level(aLevel), m_Description(std::move(aDescription)), m_Status(aStatus) {}

//------------------------------------------------------------------------------
DeviceService::Status::Status(const Status &aStatus)
    : m_Level(aStatus.m_Level), m_Description(aStatus.m_Description), m_Status(aStatus.m_Status) {}

//------------------------------------------------------------------------------
SDK::Driver::EWarningLevel::Enum DeviceService::Status::level() const {
    return m_Level;
}

//------------------------------------------------------------------------------
const QString &DeviceService::Status::description() const {
    return m_Description;
}

//------------------------------------------------------------------------------
bool DeviceService::Status::isMatched(SDK::Driver::EWarningLevel::Enum aLevel) const {
    return (m_Level <= aLevel && DSDK::getStatusType(m_Status) != DSDK::EStatus::Interface);
}

//------------------------------------------------------------------------------
DeviceService::DeviceService(IApplication *aApplication)
    : m_DeviceManager(nullptr), m_Application(aApplication), m_Log(aApplication->getLog()),
      m_DatabaseUtils(nullptr) {
    connect(&m_DetectionResult, SIGNAL(finished()), this, SLOT(onDetectionFinished()));

    m_DeviceCreationOrder[DSDK::CComponents::Watchdog] = EDeviceCreationOrder::AtStart;
    m_DeviceCreationOrder[DSDK::CComponents::Health] = EDeviceCreationOrder::AtStart;

#ifdef TC_USE_TOKEN
    m_DeviceCreationOrder[DSDK::CComponents::Token] = EDeviceCreationOrder::AtStart;
#endif
}

//------------------------------------------------------------------------------
bool DeviceService::initialize() {
    // Вытаскиваем загрузчик плагинов.
    PluginService *pluginManager = PluginService::instance(m_Application);

    m_DeviceManager = new DeviceManager(pluginManager->getPluginLoader());
    m_DeviceManager->setLog(m_Application->getLog());

    // Здесь используем DirectConnection для того, чтобы инициализация устройства при автопоиске
    // производилась из потока авто поиска и не грузила основной поток.
    connect(m_DeviceManager,
            SIGNAL(deviceDetected(const QString &, SDK::Driver::IDevice *)),
            this,
            SLOT(onDeviceDetected(const QString &, SDK::Driver::IDevice *)),
            Qt::DirectConnection);

    m_DatabaseUtils =
        DatabaseService::instance(m_Application)->getDatabaseUtils<IHardwareDatabaseUtils>();

    m_DeviceManager->initialize();

    m_IntegratedDrivers.initialize(m_DeviceManager);

    return true;
}

//------------------------------------------------------------------------------
void DeviceService::finishInitialize() {
    // Повторный эмит статусов всех устройств
    QMapIterator<QString, Status> it(m_DeviceStatusCache);

    while (it.hasNext()) {
        it.next();
        Status status = it.value();

        emit deviceStatusChanged(it.key(), status.m_Level, status.m_Description, status.m_Status);
    }

    // Создаём устройства, которым положено запускаться при старте системы
    foreach (auto deviceInstance, getConfigurations(false)) {
        QString deviceType = deviceInstance.section('.', 2, 2);

        if (m_DeviceCreationOrder.contains(deviceType) &&
            m_DeviceCreationOrder.value(deviceType) == EDeviceCreationOrder::AtStart) {
            acquireDevice(deviceInstance);
        }
    }

    LOG(m_Log, LogLevel::Normal, "DeviceService initialized.");
}

//---------------------------------------------------------------------------
bool DeviceService::canShutdown() {
    return true;
}

//------------------------------------------------------------------------------
bool DeviceService::shutdown() {
    if (m_DetectionResult.isRunning()) {
        stopDetection();

        m_DetectionResult.waitForFinished();
    }

    releaseAll();

    delete m_DeviceManager;

    return true;
}

//------------------------------------------------------------------------------
QString DeviceService::getName() const {
    return CServices::DeviceService;
}

//------------------------------------------------------------------------------
QVariantMap DeviceService::getParameters() const {
    return {};
}

//------------------------------------------------------------------------------
void DeviceService::resetParameters(const QSet<QString> & /*aParameters*/) {}

//------------------------------------------------------------------------------
const QSet<QString> &DeviceService::getRequiredServices() const {
    static QSet<QString> requiredServices = QSet<QString>() << CServices::DatabaseService
                                                            << CServices::PluginService;

    return requiredServices;
}

//------------------------------------------------------------------------------
DeviceService::~DeviceService() = default;

//------------------------------------------------------------------------------
void DeviceService::detect(const QString &aFilter) {
    m_AccessMutex.lock();

    m_DetectionResult.setFuture(QtConcurrent::run([this, aFilter]() { doDetect(aFilter); }));
}

//------------------------------------------------------------------------------
void DeviceService::doDetect(const QString &aFilter) {
    LOG(m_Log, LogLevel::Normal, "Starting device detection...");
    m_DeviceManager->detect(aFilter);
}

//------------------------------------------------------------------------------
void DeviceService::stopDetection() {
    m_DeviceManager->stopDetection();
}

//------------------------------------------------------------------------------
void DeviceService::onDetectionFinished() {
    m_AccessMutex.unlock();

    emit detectionStopped();
    LOG(m_Log, LogLevel::Normal, "Device detection is completed or terminated.");
}

//------------------------------------------------------------------------------
void DeviceService::onDeviceDetected(const QString &aConfigName, DSDK::IDevice *aDevice) {
    m_AcquiredDevices.insert(aConfigName, aDevice);

    initializeDevice(aConfigName, aDevice);

    emit deviceDetected(aConfigName);
}

//------------------------------------------------------------------------------
bool DeviceService::initializeDevice(const QString &aConfigName, DSDK::IDevice *aDevice) {
    // Устанавливаем параметры, необходимые для инициализации устройства.
    foreach (const QString &deviceType, m_InitParameters.keys()) {
        if (aConfigName.indexOf(deviceType) != -1) {
            aDevice->setDeviceConfiguration(m_InitParameters[deviceType]);
        }
    }

    // Подписываемся на события об изменении статуса устройства.
    aDevice->subscribe(
        SDK::Driver::IDevice::StatusSignal,
        this,
        SLOT(onDeviceStatus(SDK::Driver::EWarningLevel::Enum, const QString &, int)));

    // Инициализируем устройство.
    aDevice->initialize();

    // Добавляем имя устройства как параметр в БД.
    m_DatabaseUtils->setDeviceParam(
        aConfigName, PPSDK::CDatabaseConstants::Parameters::DeviceName, aDevice->getName());

    return true;
}

//------------------------------------------------------------------------------
DSDK::IDevice *DeviceService::acquireDevice(const QString &aInstancePath) {
    QMutexLocker lock(&m_AccessMutex);

    QString instancePath(aInstancePath);
    QString configInstancePath(instancePath);
    m_DeviceManager->checkITInstancePath(instancePath);

    if (m_AcquiredDevices.contains(instancePath)) {
        return m_AcquiredDevices.value(instancePath);
    }
    DSDK::IDevice *device = m_DeviceManager->acquireDevice(instancePath, configInstancePath);

    if (device) {
        m_AcquiredDevices.insert(instancePath, device);

        QMap<int, PPSDK::SKeySettings> keys = SettingsService::instance(m_Application)
                                                  ->getAdapter<PPSDK::TerminalSettings>()
                                                  ->getKeys();

        if (keys.contains(0)) {
            QVariantMap configuration;
            configuration.insert(CFiscalSDK::AutomaticNumber, keys.value(0).ap);
            device->setDeviceConfiguration(configuration);
        }

        initializeDevice(instancePath, device);
    }

    return device;
}

//------------------------------------------------------------------------------
void DeviceService::releaseDevice(DSDK::IDevice *aDevice) {
    QMutexLocker lock(&m_AccessMutex);

    m_AcquiredDevices.remove(m_AcquiredDevices.key(aDevice));

    m_DeviceManager->releaseDevice(aDevice);
}

//------------------------------------------------------------------------------
PPSDK::IDeviceService::UpdateFirmwareResult
DeviceService::updateFirmware(const QByteArray &aFirmware, const QString &aDeviceGUID) {
    // TODO: сделать проверку выше (например, в апдейтере) на то, что файл открывается и что он не
    // пуст
    QMutexLocker lock(&m_AccessMutex);

    DSDK::IDevice *device = nullptr;

    LOG(m_Log,
        LogLevel::Normal,
        QString("Start update firmware device with GUID %1.").arg(aDeviceGUID));

    for (TAcquiredDevices::iterator it = m_AcquiredDevices.begin(); it != m_AcquiredDevices.end();
         ++it) {
        if (it.key().contains(aDeviceGUID)) {
            device = it.value();
        }
    }

    if (!device) {
        LOG(m_Log,
            LogLevel::Error,
            QString("No device with GUID %1 for updating the firmware.").arg(aDeviceGUID));
        return IDeviceService::NoDevice;
    }

    if (!device->canUpdateFirmware()) {
        LOG(m_Log, LogLevel::Error, "The device cannot be updated.");
        return IDeviceService::CantUpdate;
    }

    device->updateFirmware(aFirmware);

    return IDeviceService::OK;
}

//------------------------------------------------------------------------------
QString DeviceService::createDevice(const QString &aDriverPath, const QVariantMap &aConfig) {
    QMutexLocker lock(&m_AccessMutex);

    QString driverPath(aDriverPath);
    m_IntegratedDrivers.checkDriverPath(driverPath, aConfig);

    TNamedDevice result = m_DeviceManager->createDevice(driverPath, aConfig);

    if (result.second) {
        m_AcquiredDevices.insert(result.first, result.second);

        QMap<int, PPSDK::SKeySettings> keys = SettingsService::instance(m_Application)
                                                  ->getAdapter<PPSDK::TerminalSettings>()
                                                  ->getKeys();

        if (keys.contains(0)) {
            QVariantMap configuration;
            configuration.insert(CFiscalSDK::AutomaticNumber, keys.value(0).ap);
            result.second->setDeviceConfiguration(configuration);
        }

        initializeDevice(result.first, result.second);
    } else {
        LOG(m_Log, LogLevel::Error, QString("Failed to create device %1 .").arg(driverPath));
    }

    return result.first;
}

//------------------------------------------------------------------------------
SDK::PaymentProcessor::TModelList DeviceService::getModelList(const QString &aFilter) const {
    PPSDK::TModelList result = m_DeviceManager->getModelList(aFilter);
    m_IntegratedDrivers.filterModelList(result);

    return result;
}

//------------------------------------------------------------------------------
QStringList DeviceService::getAcquiredDevicesList() const {
    QMutexLocker lock(&m_AccessMutex);

    return m_AcquiredDevices.keys();
}

//------------------------------------------------------------------------------
void DeviceService::releaseAll() {
    QMutexLocker lock(&m_AccessMutex);

    foreach (DSDK::IDevice *device, m_AcquiredDevices.values()) {
        releaseDevice(device);
    }
}

//------------------------------------------------------------------------------
QString DeviceService::getDeviceConfigName(DSDK::IDevice *aDevice) {
    QMutexLocker lock(&m_AccessMutex);

    return m_AcquiredDevices.key(aDevice);
}

//------------------------------------------------------------------------------
void DeviceService::setInitParameters(const QString &aDeviceType, const QVariantMap &aParameters) {
    m_InitParameters[aDeviceType] = aParameters;
}

//------------------------------------------------------------------------------
QStringList DeviceService::getConfigurations(bool aAllowOldConfigs) const {
    auto *settings =
        SettingsService::instance(m_Application)->getAdapter<PPSDK::TerminalSettings>();
    QStringList configurations = settings->getDeviceList();

    if (aAllowOldConfigs) {
        for (auto &configuration : configurations) {
            m_DeviceManager->checkITInstancePath(configuration);
        }
    }

    // Обязательные устройства "присутствуют" всегда
    QStringList driverList = m_DeviceManager->getDriverList();
    QStringList requiredDevices;

    requiredDevices << DSDK::CComponents::Health;
#ifdef TC_USE_TOKEN
    requiredDevices << DSDK::CComponents::Token;
#endif

    foreach (QString type, requiredDevices) {
        auto isDevicePresent = [type](const QString &aConfigurationName) -> bool {
            return aConfigurationName.contains(type);
        };

        if (std::find_if(configurations.begin(), configurations.end(), isDevicePresent) ==
                configurations.end() &&
            std::find_if(driverList.begin(), driverList.end(), isDevicePresent) !=
                driverList.end()) {
            configurations << *std::find_if(driverList.begin(), driverList.end(), isDevicePresent);
        }
    }

    return configurations;
}

//------------------------------------------------------------------------------
bool DeviceService::saveConfigurations(const QStringList &aConfigList) {
    // Проходимся по всем задействованным устройствам и сохраняем конфигурацию.
    foreach (const QString &configName, aConfigList) {
        DSDK::IDevice *device = acquireDevice(configName);

        if (device) {
            m_DeviceManager->saveConfiguration(device);
        } else {
            LOG(m_Log,
                LogLevel::Error,
                QString("Failed to set device configuration. No such device: %1.").arg(configName));
        }
    }

    auto *settings =
        SettingsService::instance(m_Application)->getAdapter<PPSDK::TerminalSettings>();
    settings->setDeviceList(aConfigList);

    m_DatabaseUtils->removeUnknownDevice(aConfigList);

    // Посылаем сигнал, чтобы остальные сервисы обновили устройства.
    emit configurationUpdated();

    return true;
}

//------------------------------------------------------------------------------
QVariantMap DeviceService::getDeviceConfiguration(const QString &aConfigName) {
    QMutexLocker lock(&m_AccessMutex);

    DSDK::IDevice *device = acquireDevice(aConfigName);

    if (device) {
        return m_DeviceManager->getDeviceConfiguration(device);
    }
    LOG(m_Log,
        LogLevel::Error,
        QString("Failed to set device configuration. No such device: %1.").arg(aConfigName));
    return {};
}

//------------------------------------------------------------------------------
void DeviceService::setDeviceConfiguration(const QString &aConfigName, const QVariantMap &aConfig) {
    DSDK::IDevice *device = acquireDevice(aConfigName);

    // Производим переинициализацию устройства.
    if (device) {
        device->release();

        {
            QMutexLocker lock(&m_AccessMutex);

            m_DeviceManager->setDeviceConfiguration(device, aConfig);
        }

        device->initialize();
    } else {
        LOG(m_Log,
            LogLevel::Error,
            QString("Failed to set device configuration. No such device: %1.").arg(aConfigName));
    }
}

//------------------------------------------------------------------------------
SDK::Plugin::TParameterList DeviceService::getDriverParameters(const QString &aDriverPath) const {
    SDK::Plugin::TParameterList result = m_DeviceManager->getDriverParameters(aDriverPath);
    m_IntegratedDrivers.filterDriverParameters(aDriverPath, result);

    return result;
}

//------------------------------------------------------------------------------
QStringList DeviceService::getDriverList() const {
    // Выдаем список драйверов всех устройств, кроме портов.
    QStringList result =
        m_DeviceManager->getDriverList().filter(QRegularExpression(".+\\.Driver\\.(?!IOPort)"));
    m_IntegratedDrivers.filterDriverList(result);

    return result;
}

//------------------------------------------------------------------------------
void DeviceService::overwriteDeviceStatus(DSDK::IDevice *aDevice,
                                          DSDK::EWarningLevel::Enum aLevel,
                                          const QString &aDescription,
                                          int aStatus) {
    // Берём последний статус и пишем в БД, только если новый статус отличается по WarningLevel
    QString configName = m_AcquiredDevices.key(aDevice);

    if (!m_DeviceStatusCache.contains(configName) ||
        m_DeviceStatusCache[configName].m_Level != aLevel ||
        DSDK::getStatusType(m_DeviceStatusCache[configName].m_Status) == DSDK::EStatus::Interface) {
        Status status(aLevel, aDescription, aStatus);

        statusChanged(aDevice, status);
    }
}

//------------------------------------------------------------------------------
void DeviceService::onDeviceStatus(DSDK::EWarningLevel::Enum aLevel,
                                   const QString &aDescription,
                                   int aStatus) {
    LogLevel::Enum logLevel = LogLevel::Warning;
    if (aLevel == DSDK::EWarningLevel::OK) {
        logLevel = LogLevel::Normal;
    } else if (aLevel == DSDK::EWarningLevel::Error) {
        logLevel = LogLevel::Error;
    }
    LOG(m_Log,
        logLevel,
        QString("Received statuses: %1, status %2").arg(aDescription).arg(aStatus));

    auto *device = dynamic_cast<DSDK::IDevice *>(sender());

    if (device) {
        Status status(aLevel, aDescription, aStatus);

        statusChanged(device, status);

        // Удаляем неиспользуемые устройства.
        foreach (auto configName, m_DeviceStatusCache.keys()) {
            if (!m_AcquiredDevices.contains(configName)) {
                m_DeviceStatusCache.remove(configName);
            }
        }
    }
}

//------------------------------------------------------------------------------
void DeviceService::statusChanged(DSDK::IDevice *aDevice, Status &aStatus) {
    DSDK::EStatus::Enum statusType = DSDK::getStatusType(aStatus.m_Status);

    // Если данный статус зарегистрирован в исключениях, то мы его не обрабатываем.
    if (statusType == DSDK::EStatus::Service) {
        return;
    }

    // Запоминаем последние статусы.
    QString configName = m_AcquiredDevices.key(aDevice);
    m_DeviceStatusCache[configName] = aStatus;

    LogLevel::Enum logLevel = (aStatus.m_Level == DSDK::EWarningLevel::OK)      ? LogLevel::Normal
                              : (aStatus.m_Level == DSDK::EWarningLevel::Error) ? LogLevel::Error
                                                                                : LogLevel::Warning;
    LOG(m_Log,
        logLevel,
        QString("Send statuses: %1, status %2, device %3")
            .arg(aStatus.m_Description)
            .arg(aStatus.m_Status)
            .arg(configName));

    if (statusType != DSDK::EStatus::Interface) {
        // Обновляем имя и информацию об устройстве в базе, т.к. возможно уточнено имя после
        // обращения к устройству.
        m_DatabaseUtils->setDeviceParam(
            configName, PPSDK::CDatabaseConstants::Parameters::DeviceName, aDevice->getName());
        QVariantMap deviceConfig = aDevice->getDeviceConfiguration();

        if (deviceConfig.contains(CHardwareSDK::DeviceData)) {
            m_DatabaseUtils->setDeviceParam(configName,
                                            PPSDK::CDatabaseConstants::Parameters::DeviceInfo,
                                            deviceConfig[CHardwareSDK::DeviceData].toString());
        }

        // Пишем статус в БД.
        m_DatabaseUtils->addDeviceStatus(configName, aStatus.m_Level, aStatus.m_Description);
    }

    emit deviceStatusChanged(configName, aStatus.m_Level, aStatus.m_Description, aStatus.m_Status);
}

//------------------------------------------------------------------------------
QSharedPointer<PPSDK::IDeviceStatus> DeviceService::getDeviceStatus(const QString &aConfigName) {
    if (m_DeviceStatusCache.contains(aConfigName)) {
        return QSharedPointer<PPSDK::IDeviceStatus>(
            new Status(m_DeviceStatusCache.value(aConfigName)));
    }

    return {nullptr};
}

//------------------------------------------------------------------------------
