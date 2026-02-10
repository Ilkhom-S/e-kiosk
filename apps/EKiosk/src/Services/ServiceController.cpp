/* @file Инициализация и получение сервисов. */

#include "Services/ServiceController.h"

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>

#include <boost/cast.hpp>

#include "Services/CryptService.h"
#include "Services/DatabaseService.h"
#include "Services/DeviceService.h"
#include "Services/EventService.h"
#include "Services/FundsService.h"
#include "Services/GUIService.h"
#include "Services/HIDService.h"
#include "Services/HookService.h"
#include "Services/NetworkService.h"
#include "Services/PaymentService.h"
#include "Services/PluginService.h"
#include "Services/PrintingService.h"
#include "Services/RemoteService.h"
#include "Services/SchedulerService.h"
#include "Services/ServiceNames.h"
#include "Services/SettingsService.h"
#include "Services/TerminalService.h"
#include "System/IApplication.h"

namespace PP = SDK::PaymentProcessor;

namespace CServiceController {
const int ShutdownRetryInterval = 1300;

const QString RestartParameters = "RESTART_PARAMETERS";
} // namespace CServiceController

//---------------------------------------------------------------------------
ServiceController::ServiceController(IApplication *aApplication)
    : m_Application(aApplication), m_FinalizeTimer(nullptr), m_ReturnCode(0) {}

//---------------------------------------------------------------------------
ServiceController::~ServiceController() {
    shutdownServices();
}

//---------------------------------------------------------------------------
void ServiceController::registerService(PP::IService *aService) {
    m_RegisteredServices.insert(aService->getName(), aService);
}

//---------------------------------------------------------------------------
void ServiceController::onEvent(const PP::Event &aEvent) {
    switch (aEvent.getType()) {
    case PPSDK::EEventType::Shutdown: {
        shutdownMachine();
        break;
    }

    case PPSDK::EEventType::Reboot: {
        rebootMachine();
        break;
    }

    case PPSDK::EEventType::Restart: {
        if (aEvent.hasData()) {
            m_UserProperties[CServiceController::RestartParameters] = aEvent.getData();
        }

        restartApplication();
        break;
    }

    case PP::EEventType::CloseApplication:
    case PP::EEventType::TerminateApplication: {
        shutdownServices();
        break;
    }

    case PP::EEventType::ReinitializeServices: {
        reinitializeServices();
        break;
    }

    // Остановка всего набора приложений. Вызывается из сервисного меню.
    case PPSDK::EEventType::StopSoftware: {
        m_ReturnCode = aEvent.getData().toMap().take("returnCode").toInt();

        auto *wsClient = TerminalService::instance(m_Application)->getClient();

        if (wsClient && wsClient->isConnected()) {
            wsClient->stopService();
        } else {
            EventService::instance(m_Application)
                ->sendEvent(PPSDK::EEventType::CloseApplication, QVariant());
        }
        break;
    }
    }
}

//---------------------------------------------------------------------------
bool ServiceController::initializeServices() {
    // Создаем EventService.
    auto *eventService = new EventService();
    eventService->initialize();

    eventService->subscribe(this, SLOT(onEvent(const SDK::PaymentProcessor::Event &)));

    // Создаем необходимые сервисы.
    registerService(eventService);
    registerService(new PrintingService(m_Application));
    registerService(new FundsService(m_Application));
    registerService(new HIDService(m_Application));
    registerService(new SettingsService(m_Application));
    registerService(new DeviceService(m_Application));
    registerService(new DatabaseService(m_Application));
    registerService(new CryptService(m_Application));
    registerService(new PluginService(m_Application));
    registerService(new NetworkService(m_Application));
    registerService(new GUIService(m_Application));
    registerService(new PaymentService(m_Application));
    registerService(new TerminalService(m_Application));
    registerService(new RemoteService(m_Application));
    registerService(new HookService(m_Application));
    registerService(new SchedulerService(m_Application));

    m_InitializedServices.clear();
    m_FailedServices.clear();
    m_ShutdownOrder.clear();

    // Запуск процесса инициализации сервисов.
    m_InitializedServices.insert(eventService->getName());
    m_ShutdownOrder.prepend(eventService);

    for (int pass = 0; pass < m_RegisteredServices.size(); pass++) {
        foreach (PP::IService *service, m_RegisteredServices) {
            // Если сервис не был инициализирован и инициализированы все зависимости, то
            // инициализируем его.
            if (!m_InitializedServices.contains(service->getName()) &&
                !m_FailedServices.contains(service->getName()) &&
                m_InitializedServices.contains(service->getRequiredServices())) {
                LOG(m_Application->getLog(),
                    LogLevel::Normal,
                    QString("Initializing %1.").arg(service->getName()));

                if (service->initialize()) {
                    LOG(m_Application->getLog(),
                        LogLevel::Normal,
                        QString("Service %1 was initialized successfully.")
                            .arg(service->getName()));

                    m_InitializedServices.insert(service->getName());
                    m_ShutdownOrder.prepend(service);
                } else {
                    m_FailedServices.insert(service->getName());
                }
            }
        }
    }

    if (m_RegisteredServices.size() == m_InitializedServices.size()) {
        foreach (PP::IService *service, m_RegisteredServices) {
            service->finishInitialize();
        }

        auto *watchServiceClient = TerminalService::instance(m_Application)->getClient();
        watchServiceClient->subscribeOnDisconnected(this);
        watchServiceClient->subscribeOnCloseCommandReceived(this);

        initializeCoreItems();

        return true;
    }

    return false;
}

//---------------------------------------------------------------------------
void ServiceController::initializeCoreItems() {
    auto *pluginLoader = PluginService::instance(m_Application)->getPluginLoader();
    QStringList corePlugins = pluginLoader->getPluginList(QRegularExpression(
        QString("PaymentProcessor\\.%1\\..*").arg(PPSDK::CComponents::CoreItem)));

    foreach (const QString &pluginName, corePlugins) {
        LOG(m_Application->getLog(),
            LogLevel::Normal,
            QString("Create core item: %1.").arg(pluginName));

        auto *plugin = pluginLoader->createPlugin(pluginName);

        if (plugin) {
            m_CorePluginList << plugin;
        }
    }
}

//---------------------------------------------------------------------------
void ServiceController::onDisconnected() {
    try {
        getEventService()->sendEvent(PPSDK::Event(PPSDK::EEventType::CloseApplication));
    } catch (std::bad_cast) {
        LOG(m_Application->getLog(),
            LogLevel::Fatal,
            "Event service was destroyed. Unable to send event 'CloseApplication' by "
            "Disconnected.");
    }
}

//---------------------------------------------------------------------------
void ServiceController::onCloseCommandReceived() {
    try {
        getEventService()->sendEvent(PPSDK::Event(PPSDK::EEventType::CloseApplication));
    } catch (std::bad_cast) {
        LOG(m_Application->getLog(),
            LogLevel::Fatal,
            "Event service was destroyed. Unable to send event 'CloseApplication' by "
            "CloseCommand.");
    }
}

//---------------------------------------------------------------------------
bool ServiceController::finalizeServices(const char *aRetrySlot) {
    auto delayFinalize = [&](const PP::IService *aService) -> bool {
        if (aService) {
            LOG(m_Application->getLog(),
                LogLevel::Warning,
                QString("Service %1 cannot be shutdown now, will try later.")
                    .arg(aService->getName()));
        }

        // Если не получилось, повторяем попытку через некоторое время.
        m_FinalizeTimer = new QTimer(this);
        m_FinalizeTimer->setSingleShot(true);
        QObject::connect(m_FinalizeTimer, SIGNAL(timeout()), this, aRetrySlot);

        m_FinalizeTimer->start(CServiceController::ShutdownRetryInterval);
        return false;
    };

    if (m_FinalizeTimer) {
        delete m_FinalizeTimer;
        m_FinalizeTimer = nullptr;
    }

    if (!canShutdown()) {
        return delayFinalize(nullptr);
    }

    finalizeCoreItems();

    // Пробуем остановить сервис.
    while (!m_ShutdownOrder.empty()) {
        PP::IService *service = m_ShutdownOrder.front();
        QString serviceName = service->getName();

        LOG(m_Application->getLog(),
            LogLevel::Normal,
            QString("Trying to shutdown service %1...").arg(serviceName));

        if (service->shutdown()) {
            m_RegisteredServices.remove(service->getName());
            m_ShutdownOrder.pop_front();
            delete service;
            service = nullptr;

            LOG(m_Application->getLog(),
                LogLevel::Debug,
                QString("Service %1 shutdown OK...").arg(serviceName));
        } else {
            return delayFinalize(service);
        }
    }

    return true;
}

//---------------------------------------------------------------------------
void ServiceController::finalizeCoreItems() {
    if (m_CorePluginList.isEmpty()) {
        return;
    }

    PluginService *ps = PluginService::instance(m_Application);

    if (ps) {
        auto *pluginLoader = ps->getPluginLoader();

        foreach (auto coreItem, m_CorePluginList) {
            LOG(m_Application->getLog(),
                LogLevel::Normal,
                QString("Destroy core item: %1.").arg(coreItem->getPluginName()));

            pluginLoader->destroyPlugin(coreItem);
        }
    }

    m_CorePluginList.clear();
}

//---------------------------------------------------------------------------
bool ServiceController::canShutdown() {
    foreach (auto service, m_ShutdownOrder) {
        if (!service->canShutdown()) {
            LOG(m_Application->getLog(),
                LogLevel::Warning,
                QString("Service %1 cannot be shutdown now, will try later.")
                    .arg(service->getName()));

            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------
void ServiceController::shutdownServices() {
    if (finalizeServices(SLOT(shutdownServices()))) {
        LOG(m_Application->getLog(), LogLevel::Normal, "Exit from ServiceController.");

        emit exit(m_ReturnCode);
    }
}

//---------------------------------------------------------------------------
void ServiceController::reinitializeServices() {
    if (finalizeServices(SLOT(reinitializeServices()))) {
        initializeServices();
    }
}

//---------------------------------------------------------------------------
void ServiceController::rebootMachine() {
    if (canShutdown()) {
        TerminalService::instance(m_Application)->getClient()->rebootMachine();
    } else {
        // Если не получилось, повторяем попытку через некоторое время.
        QTimer::singleShot(CServiceController::ShutdownRetryInterval, this, SLOT(rebootMachine()));
    }
}

//---------------------------------------------------------------------------
void ServiceController::restartApplication() {
    if (canShutdown()) {
        TerminalService::instance(m_Application)
            ->getClient()
            ->restartService(
                m_UserProperties[CServiceController::RestartParameters].toStringList());
    } else {
        // Если не получилось, повторяем попытку через некоторое время.
        QTimer::singleShot(
            CServiceController::ShutdownRetryInterval, this, SLOT(restartApplication()));
    }
}

//---------------------------------------------------------------------------
void ServiceController::shutdownMachine() {
    if (canShutdown()) {
        TerminalService::instance(m_Application)->getClient()->shutdownMachine();
    } else {
        // Если не получилось, повторяем попытку через некоторое время.
        QTimer::singleShot(
            CServiceController::ShutdownRetryInterval, this, SLOT(shutdownMachine()));
    }
}

//---------------------------------------------------------------------------
void ServiceController::dumpFailureReport() {
    QString failureInfo;
    foreach (const QString &serviceName, m_FailedServices) {
        failureInfo += QString(" ") + serviceName;
    }

    // Выводим отчет о зависимостях.
    QString details;

    foreach (const PP::IService *service, m_RegisteredServices.values()) {
        QSet<QString> requiredSet = service->getRequiredServices();

        requiredSet.intersect(m_FailedServices);

        if (!requiredSet.empty()) {
            details += "\nService " + service->getName() + " requires service ";

            // Выводим список неинициализированных зависимостей.
            foreach (const QString &serviceName, requiredSet) {
                details += serviceName + ",";
            }

            details += " initialization skipped.";
        }
    }

    LOG(m_Application->getLog(),
        LogLevel::Fatal,
        QString("Services: %1 failed to initialize due to internal errors. %2")
            .arg(failureInfo)
            .arg(details));
}

//---------------------------------------------------------------------------
QSet<SDK::PaymentProcessor::IService *> ServiceController::getServices() const {
    return {m_RegisteredServices.values().begin(), m_RegisteredServices.values().end()};
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IRemoteService *ServiceController::getRemoteService() const {
    return boost::polymorphic_cast<PP::IRemoteService *>(
        m_RegisteredServices.value(CServices::RemoteService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IPaymentService *ServiceController::getPaymentService() const {
    return boost::polymorphic_cast<PP::IPaymentService *>(
        m_RegisteredServices.value(CServices::PaymentService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IFundsService *ServiceController::getFundsService() const {
    return boost::polymorphic_cast<PP::IFundsService *>(
        m_RegisteredServices.value(CServices::FundsService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IPrinterService *ServiceController::getPrinterService() const {
    return boost::polymorphic_cast<PP::IPrinterService *>(
        m_RegisteredServices.value(CServices::PrintingService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IHIDService *ServiceController::getHIDService() const {
    return boost::polymorphic_cast<PP::IHIDService *>(
        m_RegisteredServices.value(CServices::HIDService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::INetworkService *ServiceController::getNetworkService() const {
    return boost::polymorphic_cast<PP::INetworkService *>(
        m_RegisteredServices.value(CServices::NetworkService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IEventService *ServiceController::getEventService() const {
    return boost::polymorphic_cast<PP::IEventService *>(
        m_RegisteredServices.value(CServices::EventService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IGUIService *ServiceController::getGUIService() const {
    return boost::polymorphic_cast<PP::IGUIService *>(
        m_RegisteredServices.value(CServices::GUIService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IDeviceService *ServiceController::getDeviceService() const {
    return boost::polymorphic_cast<PP::IDeviceService *>(
        m_RegisteredServices.value(CServices::DeviceService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::ICryptService *ServiceController::getCryptService() const {
    return boost::polymorphic_cast<PP::ICryptService *>(
        m_RegisteredServices.value(CServices::CryptService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::ISettingsService *ServiceController::getSettingsService() const {
    return boost::polymorphic_cast<PP::ISettingsService *>(
        m_RegisteredServices.value(CServices::SettingsService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IDatabaseService *ServiceController::getDatabaseService() const {
    return boost::polymorphic_cast<PP::IDatabaseService *>(
        m_RegisteredServices.value(CServices::DatabaseService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::ITerminalService *ServiceController::getTerminalService() const {
    return boost::polymorphic_cast<PP::ITerminalService *>(
        m_RegisteredServices.value(CServices::TerminalService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::ISchedulerService *ServiceController::getSchedulerService() const {
    return boost::polymorphic_cast<PP::ISchedulerService *>(
        m_RegisteredServices.value(CServices::SchedulerService));
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::IService *ServiceController::getService(const QString &aServiceName) const {
    if (m_RegisteredServices.isEmpty()) {
        return nullptr;
    }

    if (m_RegisteredServices.contains(aServiceName)) {
        return m_RegisteredServices.value(aServiceName);
    }

    throw PPSDK::ServiceIsNotImplemented(aServiceName);
}

//---------------------------------------------------------------------------
QVariantMap &ServiceController::getUserProperties() {
    return m_UserProperties;
}

//---------------------------------------------------------------------------
