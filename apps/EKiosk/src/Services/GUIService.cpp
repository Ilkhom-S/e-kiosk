/* @file Менеджер для работы с новым интерфейсом. */

#include "Services/GUIService.h"

#include <QtCore/QDir>
#include <QtCore/QMetaType>
#include <QtCore/QRegularExpression>
#include <QtWidgets/QApplication>

#include <SDK/GUI/IGraphicsBackend.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Payment/Step.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>

#include <SysUtils/ISysUtils.h>

#include "Services/EventService.h"
#include "Services/FirmwareUploadScenario.h"

namespace CGUIService {
const char LogName[] = "Interface";
const char IntruderLogName[] = "Penetration";
const char BackedObjectPrefix[] = "Backend$";
const char IdleScenarioName[] = "idle";
} // namespace CGUIService
#include "Services/IdleScenario.h"
#include "Services/PluginService.h"
#include "Services/ServiceNames.h"
#include "Services/TerminalService.h"
#include "System/IApplication.h"
#include "System/SettingsConstants.h"

namespace PPSDK = SDK::PaymentProcessor;

namespace CGUIService {
const int DefaultScreenWidth = 1280;
const int DefaultScreenHeight = 1024;

const int StartDragDistance = 10;

const QString DefaultScenario = "menu";
} // namespace CGUIService

//---------------------------------------------------------------------------
GUIService *GUIService::instance(IApplication *aApplication) {
    try {
        auto core = aApplication->getCore();

        if (core->getService(CServices::GUIService)) {
            return static_cast<GUIService *>(core->getGUIService());
        }
    } catch (PPSDK::ServiceIsNotImplemented) {
        return nullptr;
    }

    return nullptr;
}

//---------------------------------------------------------------------------
GUIService::GUIService(IApplication *aApplication)
    : ILogable(CGUIService::LogName), m_Application(aApplication), m_PluginService(nullptr),
      m_EventManager(nullptr), m_ScriptingCore(nullptr), m_Disabled(false), m_Width(0),
      m_Height(0) {}

//---------------------------------------------------------------------------
GUIService::~GUIService() {}

//---------------------------------------------------------------------------
bool GUIService::initialize() {
    // Выводим стандартный заголовок в лог
    getLog()->adjustPadding(-99);
    getLog()->write(LogLevel::Normal, QString("Initializing GUI Service."));
    getLog()->write(LogLevel::Normal, QString("*").repeated(58));

    m_ScriptingCore = new PPSDK::Scripting::Core(m_Application->getCore());
    m_ScriptingCore->setLog(getLog());

    m_EventManager = m_Application->getCore()->getEventService();
    m_EventManager->subscribe(this, SLOT(onEvent(const SDK::PaymentProcessor::Event &)));

    m_CheckTopmostTimer.setInterval(CGUIService::CheckTopmostWindowTimeout);
    connect(&m_CheckTopmostTimer, SIGNAL(timeout()), this, SLOT(bringToFront()));

    // Получаем директорию с файлами интерфейса из настроек.
    QString interfacePath = IApplication::toAbsolutePath(
        m_Application->getSettings().value(CSettings::InterfacePath).toString());

    connect(&m_GraphicsEngine, SIGNAL(userActivity()), &m_ScenarioEngine, SLOT(resetTimeout()));
    connect(&m_GraphicsEngine,
            SIGNAL(intruderActivity()),
            SLOT(onIntruderActivity()),
            Qt::QueuedConnection);
    connect(&m_GraphicsEngine, SIGNAL(closed()), SLOT(onMainWidgetClosed()));
    connect(&m_GraphicsEngine, SIGNAL(keyPressed(QString)), SLOT(onKeyPressed(QString)));

    m_ScenarioEngine.injectScriptObject(PPSDK::Scripting::CProxyNames::Core, m_ScriptingCore);
    m_ScenarioEngine.injectScriptObject<PPSDK::EEventType>(
        PPSDK::Scripting::CProxyNames::EventType);
    m_ScenarioEngine.injectScriptObject<PPSDK::EPaymentStep>(
        PPSDK::Scripting::CProxyNames::PaymentStep);

    m_ScenarioEngine.addDirectory(interfacePath);
    m_ScenarioEngine.addDirectory(":/Scenario");
    m_ScenarioEngine.initialize();

    m_GraphicsEngine.addContentDirectory(interfacePath);
    m_GraphicsEngine.addContentDirectory(":/GraphicsItems");

    m_GraphicsEngine.setGraphicsHost(this);

    m_PluginService = PluginService::instance(m_Application);

    loadScriptObjects();
    loadBackends();
    loadNativeScenarios();
    loadAdSources();

    QVariantMap unitedSettings;

    auto parseIni = [&](const QString &aIniFile) {
        QSettings settings(ISysUtils::rm_BOM(aIniFile), QSettings::IniFormat);

        foreach (auto key, settings.allKeys()) {
            m_Config.insert(key, settings.value(key));
        }
    };

    // Загружаем настройки интерфейса
    parseIni(interfacePath + "/interface.ini");

    // Обновляем пользовательскими настройками
    parseIni(m_Application->getUserDataPath() + "/user.ini");

    m_DefaultScenario = m_Config.value("interface/default_scenario").toString();
    if (m_DefaultScenario.isEmpty()) {
        toLog(LogLevel::Warning,
              QString("Default scenario is not specified. Set scenario name '%1'")
                  .arg(CGUIService::DefaultScenario));
        m_DefaultScenario = CGUIService::DefaultScenario;
    }

    int display = m_Config.value("interface/display", 0).toInt();
    m_Width = m_Config.value("interface/width", CGUIService::DefaultScreenWidth).toInt();
    m_Height = m_Config.value("interface/height", CGUIService::DefaultScreenHeight).toInt();

    toLog(LogLevel::Debug, QString("%1, %2, %3").arg(display).arg(m_Width).arg(m_Height));

    // Установка чувствительности для события mouse drag
    qApp->setStartDragDistance(
        m_Config.value("interface/sensitivity", CGUIService::StartDragDistance).toInt());

    bool showCursor = m_Config.value("interface/show_mouse_cursor", false).toBool();
    bool showDebugInfo = m_Config.value("interface/show_debug_info", false).toBool();

    QVariantMap scenarios = getUiSettings("scenarios");
    if (!scenarios.isEmpty()) {
        m_ExternalScenarios.clear();
        QStringList handledKeyList;

        foreach (QString key, scenarios.keys()) {
            m_ExternalScenarios.insert(scenarios.value(key).toString(), key);
            handledKeyList << scenarios.value(key).toString();
        }

        m_GraphicsEngine.addHandledKeys(handledKeyList);
    }

    if (!m_GraphicsEngine.initialize(display, m_Width, m_Height, showCursor, showDebugInfo)) {
        LOG(m_Application->getLog(), LogLevel::Error, "Failed to initialize graphics engine.");

        // GUI не будет отображаться, но платежная логика продолжит работу.
        return true;
    } else {
        // Добавляем сценарий перепрошивки устройств
        m_ScenarioEngine.addScenario(new FirmwareUploadScenario(m_Application));

        // Добавляем основной сценарий и запускаем его.
        GUI::Scenario *idle = new IdleScenario(m_Application);
        m_ScenarioEngine.addScenario(idle);

#ifndef _DEBUG
        // Запускаем проверку окна поверх всех
        m_CheckTopmostTimer.start();
#endif
        m_GraphicsEngine.start();

        QVariantMap noGui;
        noGui.insert("no_gui", m_Config.value("interface/no_gui", false).toBool());
        if (!m_ScenarioEngine.startScenario(idle->getName(), noGui)) {
            toLog(LogLevel::Error, "Failed to start idle scenario.");
            return false;
        }
    }

    m_Disabled = m_Disabled || TerminalService::instance(m_Application)->isLocked();

    return true;
}

//------------------------------------------------------------------------------
void GUIService::finishInitialize() {}

//---------------------------------------------------------------------------
bool GUIService::canShutdown() {
    return m_ScenarioEngine.canStop();
}

//---------------------------------------------------------------------------
bool GUIService::shutdown() {
    m_GraphicsEngine.stop();
    m_GraphicsEngine.finalize();
    m_ScenarioEngine.finalize();

    foreach (SDK::Plugin::IPlugin *plugin, m_BackendPluginList) {
        dynamic_cast<SDK::GUI::IGraphicsBackend *>(plugin)->shutdown();
        m_PluginService->getPluginLoader()->destroyPlugin(plugin);
    }

    m_BackendPluginList.clear();

    delete m_ScriptingCore;
    m_ScriptingCore = nullptr;

    m_EventManager->unsubscribe(this, SLOT(onEvent(const SDK::PaymentProcessor::Event &)));

    disconnect(&m_CheckTopmostTimer, SIGNAL(timeout()), this, SLOT(bringToFront()));
    disconnect(&m_GraphicsEngine, SIGNAL(userActivity()), &m_ScenarioEngine, SLOT(resetTimeout()));
    disconnect(&m_GraphicsEngine, SIGNAL(closed()), this, SLOT(onMainWidgetClosed()));
    disconnect(&m_GraphicsEngine,
               SIGNAL(keyPressed(const QString &)),
               this,
               SLOT(onKeyPressed(const QString &)));

    foreach (auto adSource, m_AdSourceList) {
        m_PluginService->getPluginLoader()->destroyPlugin(
            dynamic_cast<SDK::Plugin::IPlugin *>(adSource));
    }

    m_AdSourceList.clear();

    return true;
}

//---------------------------------------------------------------------------
QString GUIService::getName() const {
    return CServices::GUIService;
}

//---------------------------------------------------------------------------
const QSet<QString> &GUIService::getRequiredServices() const {
    static QSet<QString> requiredServices =
        QSet<QString>() << CServices::EventService << CServices::PluginService
                        << CServices::FundsService << CServices::SettingsService
                        << CServices::TerminalService << CServices::PrintingService;

    return requiredServices;
}

//---------------------------------------------------------------------------
QVariantMap GUIService::getParameters() const {
    return QVariantMap();
}

//---------------------------------------------------------------------------
void GUIService::resetParameters(const QSet<QString> &) {}

//---------------------------------------------------------------------------
QStringList GUIService::getInterfacesName() const {
    QStringList result;

    result << PPSDK::CInterfaces::ICore << PPSDK::Scripting::CProxyNames::Core;

    result.append(m_BackendScenarioObjects.keys());

    return result;
}

//---------------------------------------------------------------------------
void *GUIService::getInterface(const QString &aInterface) {
    void *object = nullptr;

    if (aInterface == PPSDK::CInterfaces::ICore) {
        object = m_Application->getCore();
    } else if (aInterface == PPSDK::Scripting::CProxyNames::Core) {
        object = m_ScriptingCore;
    } else if (m_BackendScenarioObjects.contains(aInterface)) {
        object = m_BackendScenarioObjects.value(aInterface).toStrongRef().data();
    }

    return object;
}

//---------------------------------------------------------------------------
void GUIService::onEvent(const SDK::PaymentProcessor::Event &aEvent) {
    switch (aEvent.getType()) {
    // Какое-то события сценария.
    case PPSDK::EEventType::UpdateScenario: {
        QString signal;
        QVariantMap parameters;

        if (aEvent.getData().typeId() == QMetaType::QString) {
            signal = aEvent.getData().toString();
        } else {
            parameters = aEvent.getData().value<QVariantMap>();
            signal = parameters["signal"].toString();
        }

        // FIXME: нужен другой тип события для попапов.
        if (signal == "popup_notify") {
            m_GraphicsEngine.popupNotify(signal, parameters);
        } else {
            m_ScenarioEngine.signalTriggered(signal, parameters);
        }

        break;
    }

    case PPSDK::EEventType::StartScenario: {
        // Запуск сценария. Передаем параметрами имя сценария и контекст активации (параметры
        // сценария).
        QVariantMap eventData = aEvent.getData().value<QVariantMap>();

        if (eventData.contains("name")) {
            QString scenarioName = eventData["name"].toString();

            m_ScenarioEngine.startScenario(scenarioName, eventData);
        } else {
            if (m_DefaultScenario != CGUIService::IdleScenarioName) {
                // Пытаемся запустить дефолтный сценарий.
                m_ScenarioEngine.startScenario(m_DefaultScenario);
            } else {
                // default_scenario=idle
                // idle сценарий уже запущен
            }
        }

        break;
    }

    case PPSDK::EEventType::StopScenario: {
        m_ScenarioEngine.stopScenario();
        break;
    }

    case PPSDK::EEventType::StateChanged:
        break;

    case PPSDK::EEventType::StartGraphics:
#ifndef _DEBUG
        // Запускаем проверку окна поверх всех
        m_CheckTopmostTimer.start();
#endif
        m_GraphicsEngine.start();
        break;

    case PPSDK::EEventType::PauseGraphics:
        m_GraphicsEngine.pause();
        m_CheckTopmostTimer.stop();
        break;

    case PPSDK::EEventType::StopGraphics:
        m_GraphicsEngine.stop();
        break;
    }
}

//---------------------------------------------------------------------------
void GUIService::onKeyPressed(const QString &aKeyText) {
    QString scenario = m_ExternalScenarios.value(aKeyText).toString();
    if (!scenario.isEmpty()) {
        m_ScenarioEngine.startScenario(scenario);
    }
}

//---------------------------------------------------------------------------
void GUIService::disable(bool aDisable) {
    if (m_Disabled != aDisable) {
        m_Disabled = aDisable;

        QVariantMap parameters;
        parameters["signal"] = aDisable ? CGUISignals::StopGUI : CGUISignals::StartGUI;
        EventService::instance(m_Application)
            ->sendEvent(PPSDK::EEventType::UpdateScenario, parameters);
    }
    // Каждый раз заново посылаем сигнал на disabled=true, даже если уже заблокирован, т.к. причина
    // блокировки могла измениться
    else if (m_Disabled) {
        QVariantMap parameters;
        parameters["signal"] = CGUISignals::UpdateGUI;
        EventService::instance(m_Application)
            ->sendEvent(PPSDK::EEventType::UpdateScenario, parameters);
    }
}

//---------------------------------------------------------------------------
void GUIService::onHIDData(const QVariant &aData) {
    // TODO
    QVariantMap arguments;
    arguments["msisdn"] = aData;

    m_ScenarioEngine.signalTriggered("datapending", arguments);
}

//---------------------------------------------------------------------------
bool GUIService::show(const QString &aScene, const QVariantMap &aParameters) {
    return m_GraphicsEngine.show(aScene, aParameters);
}

//---------------------------------------------------------------------------
bool GUIService::showPopup(const QString &aWidget, const QVariantMap &aParameters) {
    return m_GraphicsEngine.showPopup(aWidget, aParameters);
}

//---------------------------------------------------------------------------
QVariantMap GUIService::showModal(const QString &aWidget, const QVariantMap &aParameters) {
    return m_GraphicsEngine.showModal(aWidget, aParameters);
}

//---------------------------------------------------------------------------
bool GUIService::hidePopup(const QVariantMap &aParameters) {
    return m_GraphicsEngine.hidePopup(aParameters);
}

//---------------------------------------------------------------------------
void GUIService::notify(const QString &aEvent, const QVariantMap &aParameters) {
    m_GraphicsEngine.notify(aEvent, aParameters);
}

//---------------------------------------------------------------------------
void GUIService::onMainWidgetClosed() {
    m_EventManager->sendEvent(PPSDK::Event(PPSDK::EEventType::CloseApplication));
}

//---------------------------------------------------------------------------
void GUIService::onIntruderActivity() {
    auto settings = dynamic_cast<PPSDK::TerminalSettings *>(
                        m_Application->getCore()->getSettingsService()->getAdapter(
                            PPSDK::CAdapterNames::TerminalAdapter))
                        ->getCommonSettings();

    auto event = PPSDK::EEventType::OK;
    auto message = tr("#penetration_detected");

    switch (settings.penetrationEventLevel) {
    case PPSDK::EEventType::Critical:
        event = settings.penetrationEventLevel;
        message += " #alarm";

        if (settings.blockOn(PPSDK::SCommonSettings::Penetration)) {
            m_EventManager->sendEvent(
                PPSDK::Event(PPSDK::EEventType::TerminalLock, CGUIService::LogName, message));
        }
        break;

    case PPSDK::EEventType::Warning:
        message += " #alarm";
        break;
    }

#ifndef _DEBUG
    ILog::getInstance(CGUIService::IntruderLogName)->write(LogLevel::Warning, message);
#endif
}

//---------------------------------------------------------------------------
bool GUIService::isDisabled() const {
    return m_Disabled;
}

//---------------------------------------------------------------------------
void GUIService::reset() {
    m_GraphicsEngine.reset();
}

//---------------------------------------------------------------------------
QRect GUIService::getScreenSize(int aIndex) const {
    return aIndex ? m_GraphicsEngine.getDisplayRectangle(aIndex) : QRect(0, 0, m_Width, m_Height);
}

//---------------------------------------------------------------------------
QPixmap GUIService::getScreenshot() {
    return m_GraphicsEngine.getScreenshot();
}

//---------------------------------------------------------------------------
QVariantMap GUIService::getUiSettings(const QString &aSection) const {
    QVariantMap result;

    foreach (QString key, m_Config.keys()) {
        if (!key.contains(aSection)) {
            continue;
        }

        result.insert(key.split("/").last(), m_Config.value(key));
    }

    return result;
}

//---------------------------------------------------------------------------
void GUIService::loadAdSources() {
    QStringList adSources = m_PluginService->getPluginLoader()->getPluginList(
        QRegularExpression("PaymentProcessor\\.AdSource\\..*"));

    foreach (const QString &source, adSources) {
        auto plugin = m_PluginService->getPluginLoader()->createPlugin(source);
        auto adSource = dynamic_cast<SDK::GUI::IAdSource *>(plugin);

        if (adSource) {
            m_AdSourceList << adSource;
        } else {
            m_PluginService->getPluginLoader()->destroyPlugin(plugin);
        }
    }
}

//---------------------------------------------------------------------------
void GUIService::loadNativeScenarios() {
    QStringList scenarios = m_PluginService->getPluginLoader()->getPluginList(
        QRegularExpression("PaymentProcessor\\.ScenarioFactory\\..*"));

    foreach (const QString &scenario, scenarios) {
        auto plugin = m_PluginService->getPluginLoader()->createPlugin(scenario);
        auto factory = dynamic_cast<SDK::Plugin::IFactory<GUI::Scenario> *>(plugin);

        if (factory) {
            // Создаем сценарии.
            foreach (auto className, factory->getClassNames()) {
                GUI::Scenario *scenarioObject = factory->create(className);
                m_ScenarioEngine.addScenario(scenarioObject);
            }
        } else {
            LOG(m_Application->getLog(),
                LogLevel::Error,
                QString("Bad scenario plugin %1.").arg(scenario));
        }

        m_PluginService->getPluginLoader()->destroyPlugin(plugin);
    }
}

//---------------------------------------------------------------------------
void GUIService::loadBackends() {
    QStringList backends = m_PluginService->getPluginLoader()->getPluginList(
        QRegularExpression("PaymentProcessor\\.GraphicsBackend\\..*"));

    foreach (const QString &backend, backends) {
        SDK::Plugin::IPlugin *plugin = m_PluginService->getPluginLoader()->createPlugin(backend);

        SDK::GUI::IGraphicsBackend *backendObject =
            dynamic_cast<SDK::GUI::IGraphicsBackend *>(plugin);
        if (backendObject) {
            backendObject->initialize(&m_GraphicsEngine);
            m_GraphicsEngine.addBackend(backendObject);

            m_BackendPluginList << plugin;
        } else {
            LOG(m_Application->getLog(),
                LogLevel::Error,
                QString("Bad backend plugin %1.").arg(backend));
            m_PluginService->getPluginLoader()->destroyPlugin(plugin);
        }
    }
}

//---------------------------------------------------------------------------
void GUIService::loadScriptObjects() {
    QStringList scriptObjects = m_PluginService->getPluginLoader()->getPluginList(
        QRegularExpression("PaymentProcessor\\.ScriptFactory\\..*"));

    foreach (const QString &scriptPluginName, scriptObjects) {
        auto plugin = m_PluginService->getPluginLoader()->createPlugin(scriptPluginName);
        auto factory =
            dynamic_cast<SDK::Plugin::IFactory<PPSDK::Scripting::IBackendScenarioObject> *>(plugin);

        if (factory) {
            foreach (auto className, factory->getClassNames()) {
                PPSDK::Scripting::IBackendScenarioObject *scriptObject = factory->create(className);

                toLog(LogLevel::Normal,
                      QString("Register scenario backend object '%1' from '%2'.")
                          .arg(scriptObject->getName())
                          .arg(scriptPluginName));

                QString objectName = CGUIService::BackedObjectPrefix + scriptObject->getName();
                m_ScenarioEngine.injectScriptObject(objectName, scriptObject);
                // TODO PORT_QT5
                // m_BackendScenarioObjects.insert(objectName, QWeakPointer<QObject>(scriptObject));
            }
        } else {
            LOG(m_Application->getLog(),
                LogLevel::Error,
                QString("Bad script object plugin %1.").arg(scriptPluginName));
            m_PluginService->getPluginLoader()->destroyPlugin(plugin);
        }
    }
}

//---------------------------------------------------------------------------
SDK::GUI::IAdSource *GUIService::getAdSource() const {
    return m_AdSourceList.count() ? m_AdSourceList.first() : nullptr;
}

//------------------------------------------------------------------------
QObject *GUIService::getBackendObject(const QString &aName) const {
    QString fullName = CGUIService::BackedObjectPrefix + aName;
    return m_BackendScenarioObjects.keys().contains(fullName)
               ? m_BackendScenarioObjects.value(fullName).toStrongRef().data()
               : nullptr;
}

//------------------------------------------------------------------------
void GUIService::bringToFront() {
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        if (!widget->isHidden()) {
            ISysUtils::bringWindowToFront(widget->winId());
        }
    }

    auto getTopmostWindowsTitle = [](QSettings &aSettings) -> QStringList {
        QStringList topmostWindows;

        aSettings.beginGroup("topmost");
        foreach (auto const key, aSettings.allKeys()) {
            QVariant v = aSettings.value(key);
            switch (v.typeId()) {
            case QMetaType::QStringList:
                topmostWindows.append(v.toStringList());
                break;
            case QMetaType::QString:
                topmostWindows.push_back(v.toString());
                break;
            }
        }
        aSettings.endGroup();

        return topmostWindows;
    };

    static QStringList topmostWindows = getTopmostWindowsTitle(m_Application->getSettings());

    foreach (auto title, topmostWindows) {
        ISysUtils::bringWindowToFront(title);
    }
}

//---------------------------------------------------------------------------
