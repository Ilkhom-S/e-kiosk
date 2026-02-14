/* @file Сервис для работы с настройками. */

#include "Services/SettingsService.h"

#include <QtCore/QDir>

#include <Common/Version.h>

#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Settings/Directory.h>
#include <SDK/PaymentProcessor/Settings/ExtensionsSettings.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Settings/UserSettings.h>

#include <SettingsManager/SettingsManager.h>
#include <SysUtils/ISysUtils.h>
#include <cstdio>

#include "Services/EventService.h"
#include "Services/ServiceNames.h"
#include "System/IApplication.h"
#include "System/SettingsConstants.h"

namespace PPSDK = SDK::PaymentProcessor;
namespace AdapterNames = PPSDK::CAdapterNames;

//---------------------------------------------------------------------------
SettingsService *SettingsService::instance(IApplication *aApplication) {
    return dynamic_cast<SettingsService *>(
        aApplication->getCore()->getService(CServices::SettingsService));
}

//---------------------------------------------------------------------------
SettingsService::SettingsService(IApplication *aApplication)
    : m_Application(aApplication), m_SettingsManager(nullptr), m_RestoreConfiguration(false) {}

//---------------------------------------------------------------------------
SettingsService::~SettingsService() = default;

//---------------------------------------------------------------------------
bool SettingsService::initialize() {
    m_SettingsManager = new SettingsManager(m_Application->getUserDataPath());

    m_SettingsManager->setLog(m_Application->getLog());

    //---------------------------------------------------------------------------
    // Регистрируем все конфиги.
    // ВАЖНО: Порядок файлов определяет приоритет настроек.
    // Файлы, добавленные позже (например, user.ini), могут переопределять значения из файлов,
    // добавленных раньше (например, system.ini). Это позволяет реализовать систему глобальных
    // (system.ini) и пользовательских (user.ini) настроек с возможностью переопределения. Например,
    // если ключ присутствует и в system.ini, и в user.ini, будет использоваться значение из
    // user.ini. Такой подход обеспечивает гибкость и удобство конфигурирования.
    QList<SSettingsSource> settingsSources;
    settingsSources << SSettingsSource(ISysUtils::rm_BOM(IApplication::getWorkingDirectory() +
                                                         "/data/system.ini"),
                                       AdapterNames::TerminalAdapter,
                                       true)
                    << SSettingsSource("terminal.ini", AdapterNames::TerminalAdapter, false)
                    << SSettingsSource("keys.xml", AdapterNames::TerminalAdapter, false)
                    << SSettingsSource("config.xml", AdapterNames::TerminalAdapter, true)

                    << SSettingsSource("commissions.xml", AdapterNames::DealerAdapter, true)
                    << SSettingsSource("commissions.local.xml", AdapterNames::DealerAdapter, true)

                    << SSettingsSource("config.xml", AdapterNames::DealerAdapter, true)
                    << SSettingsSource("customers.xml", AdapterNames::DealerAdapter, true)
                    << SSettingsSource("groups.xml", AdapterNames::DealerAdapter, true)

                    // user.ini должен идти после system.ini, чтобы пользовательские настройки могли
                    // переопределять системные.
                    << SSettingsSource("user.ini", AdapterNames::UserAdapter, true)

                    << SSettingsSource("numcapacity.xml", AdapterNames::Directory, true)
                    << SSettingsSource(IApplication::getWorkingDirectory() + "/data/directory.xml",
                                       AdapterNames::Directory,
                                       true)
                    << SSettingsSource("config.xml", AdapterNames::Extensions, true)
                    /* Временный конфиг #38560 */
                    << SSettingsSource("extensions.xml", AdapterNames::Extensions, true);

    // Загрузка всех operators.xml
    foreach (auto file,
             QDir(m_Application->getUserDataPath())
                 .entryInfoList(QStringList() << "operators*.xml", QDir::Files, QDir::Name)) {
        // Вставляем в property_tree как ссылку на файл
        settingsSources << SSettingsSource(
            file.filePath(), AdapterNames::DealerAdapter, "operators");
    }

    // Загружаем настройки.
    m_SettingsManager->loadSettings(settingsSources);

    // Инициализируем все адаптеры настроек.
    auto *terminalSettings =
        new SDK::PaymentProcessor::TerminalSettings(m_SettingsManager->getProperties());
    terminalSettings->setLog(m_Application->getLog());
    terminalSettings->initialize();

    // Устанавливаем переменные окружения.
    SDK::PaymentProcessor::SAppEnvironment environment;
    environment.userDataPath = IApplication::toAbsolutePath(
        m_Application->getSettings().value(CSettings::UserDataPath).toString());
    environment.contentPath = IApplication::toAbsolutePath(
        m_Application->getSettings().value(CSettings::ContentPath).toString());
    environment.interfacePath = IApplication::toAbsolutePath(
        m_Application->getSettings().value(CSettings::InterfacePath).toString());
    environment.adPath = IApplication::toAbsolutePath(
        m_Application->getSettings().value(CSettings::AdPath).toString());
    environment.version = Humo::getVersion();

    terminalSettings->setAppEnvironment(environment);

    auto *dealerSettings =
        new SDK::PaymentProcessor::DealerSettings(m_SettingsManager->getProperties());
    dealerSettings->setLog(m_Application->getLog());
    dealerSettings->initialize();

    auto *directory = new SDK::PaymentProcessor::Directory(m_SettingsManager->getProperties());
    directory->setLog(m_Application->getLog());

    auto *extensionsSettings =
        new SDK::PaymentProcessor::ExtensionsSettings(m_SettingsManager->getProperties());
    extensionsSettings->setLog(m_Application->getLog());

    auto *userSettings =
        new SDK::PaymentProcessor::UserSettings(m_SettingsManager->getProperties());
    userSettings->setLog(m_Application->getLog());

    m_SettingsAdapters.insert(AdapterNames::TerminalAdapter, terminalSettings);

    m_SettingsAdapters.insert(AdapterNames::DealerAdapter, dealerSettings);
    m_SettingsAdapters.insert(AdapterNames::UserAdapter, userSettings);
    m_SettingsAdapters.insert(AdapterNames::Directory, directory);
    m_SettingsAdapters.insert(AdapterNames::Extensions, extensionsSettings);

    return true;
}

//------------------------------------------------------------------------------
void SettingsService::finishInitialize() {
    if (m_RestoreConfiguration) {
        EventService::instance(m_Application)
            ->sendEvent(PPSDK::EEventType::RestoreConfiguration, QVariant());

        m_RestoreConfiguration = false;
    }
}

//---------------------------------------------------------------------------
bool SettingsService::canShutdown() {
    return true;
}

//---------------------------------------------------------------------------
bool SettingsService::shutdown() {
    foreach (SDK::PaymentProcessor::ISettingsAdapter *adapter, m_SettingsAdapters) {
        delete adapter;
    }

    m_SettingsAdapters.clear();
    delete m_SettingsManager;
    return true;
}

//---------------------------------------------------------------------------
QString SettingsService::getName() const {
    return CServices::SettingsService;
}

//---------------------------------------------------------------------------
const QSet<QString> &SettingsService::getRequiredServices() const {
    static QSet<QString> requiredServices;
    return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap SettingsService::getParameters() const {
    return {};
}

//---------------------------------------------------------------------------
void SettingsService::resetParameters(const QSet<QString> & /*aParameters*/) {}

//---------------------------------------------------------------------------
SettingsManager *SettingsService::getSettingsManager() const {
    return m_SettingsManager;
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::ISettingsAdapter *SettingsService::getAdapter(const QString &aAdapterName) {
    if (m_SettingsAdapters.contains(aAdapterName)) {
        return m_SettingsAdapters[aAdapterName];
    } else {
        return nullptr;
    }
}

//---------------------------------------------------------------------------
bool SettingsService::saveConfiguration() {
    return m_SettingsManager->saveSettings();
}

//---------------------------------------------------------------------------
QList<SDK::PaymentProcessor::ISettingsAdapter *> SettingsService::enumerateAdapters() const {
    return m_SettingsAdapters.values();
}

//---------------------------------------------------------------------------
