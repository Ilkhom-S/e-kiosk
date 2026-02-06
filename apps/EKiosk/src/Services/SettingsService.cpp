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

#include "Services/EventService.h"
#include "Services/ServiceNames.h"
#include "System/IApplication.h"
#include "System/SettingsConstants.h"

namespace PPSDK = SDK::PaymentProcessor;
namespace AdapterNames = PPSDK::CAdapterNames;

//---------------------------------------------------------------------------
SettingsService *SettingsService::instance(IApplication *aApplication) {
    return static_cast<SettingsService *>(
        aApplication->getCore()->getService(CServices::SettingsService));
}

//---------------------------------------------------------------------------
SettingsService::SettingsService(IApplication *aApplication)
    : mApplication(aApplication), mSettingsManager(nullptr), mRestoreConfiguration(false) {}

//---------------------------------------------------------------------------
SettingsService::~SettingsService() {}

//---------------------------------------------------------------------------
bool SettingsService::initialize() {
    mSettingsManager = new SettingsManager(mApplication->getUserDataPath());

    mSettingsManager->setLog(mApplication->getLog());

    //---------------------------------------------------------------------------
    // Регистрируем все конфиги.
    // ВАЖНО: Порядок файлов определяет приоритет настроек.
    // Файлы, добавленные позже (например, user.ini), могут переопределять значения из файлов,
    // добавленных раньше (например, system.ini). Это позволяет реализовать систему глобальных
    // (system.ini) и пользовательских (user.ini) настроек с возможностью переопределения. Например,
    // если ключ присутствует и в system.ini, и в user.ini, будет использоваться значение из
    // user.ini. Такой подход обеспечивает гибкость и удобство конфигурирования.
    QList<SSettingsSource> settingsSources;
    settingsSources << SSettingsSource(ISysUtils::rmBOM(mApplication->getWorkingDirectory() +
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
                    << SSettingsSource(mApplication->getWorkingDirectory() + "/data/directory.xml",
                                       AdapterNames::Directory,
                                       true)
                    << SSettingsSource("config.xml", AdapterNames::Extensions, true)
                    /* Временный конфиг #38560 */
                    << SSettingsSource("extensions.xml", AdapterNames::Extensions, true);

    // Загрузка всех operators.xml
    foreach (auto file,
             QDir(mApplication->getUserDataPath())
                 .entryInfoList(QStringList() << "operators*.xml", QDir::Files, QDir::Name)) {
        // Вставляем в property_tree как ссылку на файл
        settingsSources << SSettingsSource(
            file.filePath(), AdapterNames::DealerAdapter, "operators");
    }

    // Загружаем настройки.
    mSettingsManager->loadSettings(settingsSources);

    // Инициализируем все адаптеры настроек.
    SDK::PaymentProcessor::TerminalSettings *terminalSettings =
        new SDK::PaymentProcessor::TerminalSettings(mSettingsManager->getProperties());
    terminalSettings->setLog(mApplication->getLog());
    terminalSettings->initialize();

    // Устанавливаем переменные окружения.
    SDK::PaymentProcessor::SAppEnvironment environment;
    environment.userDataPath = IApplication::toAbsolutePath(
        mApplication->getSettings().value(CSettings::UserDataPath).toString());
    environment.contentPath = IApplication::toAbsolutePath(
        mApplication->getSettings().value(CSettings::ContentPath).toString());
    environment.interfacePath = IApplication::toAbsolutePath(
        mApplication->getSettings().value(CSettings::InterfacePath).toString());
    environment.adPath = IApplication::toAbsolutePath(
        mApplication->getSettings().value(CSettings::AdPath).toString());
    environment.version = Humo::getVersion();

    terminalSettings->setAppEnvironment(environment);

    SDK::PaymentProcessor::DealerSettings *dealerSettings =
        new SDK::PaymentProcessor::DealerSettings(mSettingsManager->getProperties());
    dealerSettings->setLog(mApplication->getLog());
    dealerSettings->initialize();

    SDK::PaymentProcessor::Directory *directory =
        new SDK::PaymentProcessor::Directory(mSettingsManager->getProperties());
    directory->setLog(mApplication->getLog());

    SDK::PaymentProcessor::ExtensionsSettings *extensionsSettings =
        new SDK::PaymentProcessor::ExtensionsSettings(mSettingsManager->getProperties());
    extensionsSettings->setLog(mApplication->getLog());

    SDK::PaymentProcessor::UserSettings *userSettings =
        new SDK::PaymentProcessor::UserSettings(mSettingsManager->getProperties());
    userSettings->setLog(mApplication->getLog());

    mSettingsAdapters.insert(AdapterNames::TerminalAdapter, terminalSettings);
    mSettingsAdapters.insert(AdapterNames::DealerAdapter, dealerSettings);
    mSettingsAdapters.insert(AdapterNames::UserAdapter, userSettings);
    mSettingsAdapters.insert(AdapterNames::Directory, directory);
    mSettingsAdapters.insert(AdapterNames::Extensions, extensionsSettings);

    return true;
}

//------------------------------------------------------------------------------
void SettingsService::finishInitialize() {
    if (mRestoreConfiguration) {
        EventService::instance(mApplication)
            ->sendEvent(PPSDK::EEventType::RestoreConfiguration, QVariant());

        mRestoreConfiguration = false;
    }
}

//---------------------------------------------------------------------------
bool SettingsService::canShutdown() {
    return true;
}

//---------------------------------------------------------------------------
bool SettingsService::shutdown() {
    foreach (SDK::PaymentProcessor::ISettingsAdapter *adapter, mSettingsAdapters) {
        delete adapter;
    }

    mSettingsAdapters.clear();
    delete mSettingsManager;
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
    return QVariantMap();
}

//---------------------------------------------------------------------------
void SettingsService::resetParameters(const QSet<QString> &) {}

//---------------------------------------------------------------------------
SettingsManager *SettingsService::getSettingsManager() const {
    return mSettingsManager;
}

//---------------------------------------------------------------------------
SDK::PaymentProcessor::ISettingsAdapter *SettingsService::getAdapter(const QString &aAdapterName) {
    return mSettingsAdapters.contains(aAdapterName) ? mSettingsAdapters[aAdapterName] : 0;
}

//---------------------------------------------------------------------------
bool SettingsService::saveConfiguration() {
    return mSettingsManager->saveSettings();
}

//---------------------------------------------------------------------------
QList<SDK::PaymentProcessor::ISettingsAdapter *> SettingsService::enumerateAdapters() const {
    return mSettingsAdapters.values();
}

//---------------------------------------------------------------------------
