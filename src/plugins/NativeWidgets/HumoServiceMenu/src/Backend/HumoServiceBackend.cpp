/* @file Бэкэнд для HumoService */

#include "HumoServiceBackend.h"

#include <QtCore/QRegularExpression>
#include <QtCore/QTimer>

#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/IPluginEnvironment.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <SDK/Plugins/PluginInitializer.h>

#include "HardwareManager.h"
#include "KeysManager.h"
#include "NetworkManager.h"
#include "PaymentManager.h"

HumoServiceBackend::HumoServiceBackend(SDK::Plugin::IEnvironment *aFactory, ILog *aLog)
    : m_Factory(aFactory), m_Log(aLog), m_TerminalSettings(nullptr), m_AutoEncashmentEnabled(false),
      m_AuthorizationEnabled(false) {
    m_Core = dynamic_cast<SDK::PaymentProcessor::ICore *>(
        m_Factory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

    if (m_Core) {
        m_TerminalSettings = static_cast<SDK::PaymentProcessor::TerminalSettings *>(
            m_Core->getSettingsService()->getAdapter(
                SDK::PaymentProcessor::CAdapterNames::TerminalAdapter));
    }

    // Initialize managers
    m_HardwareManager = QSharedPointer<HardwareManager>(new HardwareManager(m_Factory, m_Core));
    m_KeysManager = QSharedPointer<KeysManager>(new KeysManager(m_Core));
    m_NetworkManager = QSharedPointer<NetworkManager>(new NetworkManager(m_Core));
    m_PaymentManager = QSharedPointer<PaymentManager>(new PaymentManager(m_Core));

    // Setup heartbeat timer
    connect(&m_HeartbeatTimer, &QTimer::timeout, this, &HumoServiceBackend::sendHeartbeat);
    m_HeartbeatTimer.setInterval(CHumoServiceBackend::HeartbeatTimeout);
}

//--------------------------------------------------------------------------
HumoServiceBackend::~HumoServiceBackend() {
    stopHeartbeat();
}

//--------------------------------------------------------------------------
bool HumoServiceBackend::authorize(const QString &aPassword) {
    // TODO: Implement authorization logic
    return true;
}

//--------------------------------------------------------------------------
HumoServiceBackend::TAccessRights HumoServiceBackend::getAccessRights() const {
    return m_AccessRights;
}

//--------------------------------------------------------------------------
bool HumoServiceBackend::isAuthorizationEnabled() const {
    return m_AuthorizationEnabled;
}

//--------------------------------------------------------------------------
bool HumoServiceBackend::isConfigurationChanged() {
    // TODO: Implement configuration change detection
    return false;
}

//--------------------------------------------------------------------------
HardwareManager *HumoServiceBackend::getHardwareManager() {
    return m_HardwareManager.data();
}

//--------------------------------------------------------------------------
KeysManager *HumoServiceBackend::getKeysManager() {
    return m_KeysManager.data();
}

//--------------------------------------------------------------------------
NetworkManager *HumoServiceBackend::getNetworkManager() {
    return m_NetworkManager.data();
}

//--------------------------------------------------------------------------
PaymentManager *HumoServiceBackend::getPaymentManager() {
    return m_PaymentManager.data();
}

//--------------------------------------------------------------------------
void HumoServiceBackend::toLog(const QString &aMessage) {
    toLog(LogLevel::Normal, aMessage);
}

//--------------------------------------------------------------------------
void HumoServiceBackend::toLog(LogLevel::Enum aLevel, const QString &aMessage) {
    if (m_Log) {
        m_Log->write(aLevel, aMessage);
    }
}

//--------------------------------------------------------------------------
SDK::PaymentProcessor::ICore *HumoServiceBackend::getCore() const {
    return m_Core;
}

//--------------------------------------------------------------------------
void HumoServiceBackend::getTerminalInfo(QVariantMap &aTerminalInfo) {
    // TODO: Implement terminal info retrieval
}

//--------------------------------------------------------------------------
void HumoServiceBackend::sendEvent(SDK::PaymentProcessor::EEventType::Enum aEventType) {
    sendEvent(aEventType, QVariantMap());
}

//--------------------------------------------------------------------------
void HumoServiceBackend::sendEvent(SDK::PaymentProcessor::EEventType::Enum aEventType,
                                   const QVariantMap &aParameters) {
    if (m_Core) {
        m_Core->getEventService()->sendEvent(
            SDK::PaymentProcessor::Event(aEventType, "HumoServiceBackend", aParameters));
    }
}

//--------------------------------------------------------------------------
bool HumoServiceBackend::saveConfiguration() {
    // TODO: Implement configuration saving
    return true;
}

//--------------------------------------------------------------------------
void HumoServiceBackend::setConfiguration(const QVariantMap &aParameters) {
    m_Parameters = aParameters;
}

//--------------------------------------------------------------------------
QVariantMap HumoServiceBackend::getConfiguration() const {
    return m_Parameters;
}

//--------------------------------------------------------------------------
void HumoServiceBackend::saveDispenserUnitState() {
    // TODO: Implement dispenser state saving
}

//--------------------------------------------------------------------------
void HumoServiceBackend::printDispenserDiffState() {
    // TODO: Implement dispenser diff printing
}

//--------------------------------------------------------------------------
void HumoServiceBackend::needUpdateConfigs() {
    // TODO: Implement config update logic
}

//--------------------------------------------------------------------------
bool HumoServiceBackend::hasAnyPassword() const {
    // TODO: Implement password checking
    return false;
}

//--------------------------------------------------------------------------
QList<QWidget *> HumoServiceBackend::getExternalWidgets(bool aReset) {
    QStringList plugins = m_Factory->getPluginLoader()->getPluginList(
        QRegularExpression("PaymentProcessor\\.Application\\.ServiceMenu\\..*"));

    if (m_WidgetPluginList.isEmpty()) {
        foreach (const QString &widget, plugins) {
            SDK::Plugin::IPlugin *plugin = m_Factory->getPluginLoader()->createPlugin(widget);
            if (plugin) {
                m_WidgetPluginList << plugin;
            }
        }
    }

    QList<QWidget *> widgetList;

    foreach (SDK::Plugin::IPlugin *plugin, m_WidgetPluginList) {
        SDK::GUI::IGraphicsItem *item_Object = dynamic_cast<SDK::GUI::IGraphicsItem *>(plugin);
        if (item_Object) {
            if (aReset) {
                item_Object->reset(QVariantMap());
            }

            QWidget *widget = item_Object->getNativeWidget();
            if (widget) {
                widgetList << widget;
            }
        }
    }

    return widgetList;
}

//--------------------------------------------------------------------------
void HumoServiceBackend::startHeartbeat() {
    if (!m_HeartbeatTimer.isActive()) {
        m_HeartbeatTimer.start();
    }
}

//--------------------------------------------------------------------------
void HumoServiceBackend::stopHeartbeat() {
    if (m_HeartbeatTimer.isActive()) {
        m_HeartbeatTimer.stop();
    }
}

//--------------------------------------------------------------------------
void HumoServiceBackend::sendHeartbeat() {
    // TODO: Implement heartbeat sending
}

//--------------------------------------------------------------------------