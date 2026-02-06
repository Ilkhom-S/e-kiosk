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
    : mFactory(aFactory), mLog(aLog), mTerminalSettings(nullptr), mAutoEncashmentEnabled(false),
      mAuthorizationEnabled(false) {
    mCore = dynamic_cast<SDK::PaymentProcessor::ICore *>(
        mFactory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

    if (mCore) {
        mTerminalSettings = static_cast<SDK::PaymentProcessor::TerminalSettings *>(
            mCore->getSettingsService()->getAdapter(
                SDK::PaymentProcessor::CAdapterNames::TerminalAdapter));
    }

    // Initialize managers
    mHardwareManager = QSharedPointer<HardwareManager>(new HardwareManager(mFactory, mCore));
    mKeysManager = QSharedPointer<KeysManager>(new KeysManager(mCore));
    mNetworkManager = QSharedPointer<NetworkManager>(new NetworkManager(mCore));
    mPaymentManager = QSharedPointer<PaymentManager>(new PaymentManager(mCore));

    // Setup heartbeat timer
    connect(&mHeartbeatTimer, &QTimer::timeout, this, &HumoServiceBackend::sendHeartbeat);
    mHeartbeatTimer.setInterval(CHumoServiceBackend::HeartbeatTimeout);
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
    return mAccessRights;
}

//--------------------------------------------------------------------------
bool HumoServiceBackend::isAuthorizationEnabled() const {
    return mAuthorizationEnabled;
}

//--------------------------------------------------------------------------
bool HumoServiceBackend::isConfigurationChanged() {
    // TODO: Implement configuration change detection
    return false;
}

//--------------------------------------------------------------------------
HardwareManager *HumoServiceBackend::getHardwareManager() {
    return mHardwareManager.data();
}

//--------------------------------------------------------------------------
KeysManager *HumoServiceBackend::getKeysManager() {
    return mKeysManager.data();
}

//--------------------------------------------------------------------------
NetworkManager *HumoServiceBackend::getNetworkManager() {
    return mNetworkManager.data();
}

//--------------------------------------------------------------------------
PaymentManager *HumoServiceBackend::getPaymentManager() {
    return mPaymentManager.data();
}

//--------------------------------------------------------------------------
void HumoServiceBackend::toLog(const QString &aMessage) {
    toLog(LogLevel::Normal, aMessage);
}

//--------------------------------------------------------------------------
void HumoServiceBackend::toLog(LogLevel::Enum aLevel, const QString &aMessage) {
    if (mLog) {
        mLog->write(aLevel, aMessage);
    }
}

//--------------------------------------------------------------------------
SDK::PaymentProcessor::ICore *HumoServiceBackend::getCore() const {
    return mCore;
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
    if (mCore) {
        mCore->getEventService()->sendEvent(
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
    mParameters = aParameters;
}

//--------------------------------------------------------------------------
QVariantMap HumoServiceBackend::getConfiguration() const {
    return mParameters;
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
    QStringList plugins = mFactory->getPluginLoader()->getPluginList(
        QRegularExpression("PaymentProcessor\\.Application\\.ServiceMenu\\..*"));

    if (mWidgetPluginList.isEmpty()) {
        foreach (const QString &widget, plugins) {
            SDK::Plugin::IPlugin *plugin = mFactory->getPluginLoader()->createPlugin(widget);
            if (plugin) {
                mWidgetPluginList << plugin;
            }
        }
    }

    QList<QWidget *> widgetList;

    foreach (SDK::Plugin::IPlugin *plugin, mWidgetPluginList) {
        SDK::GUI::IGraphicsItem *itemObject = dynamic_cast<SDK::GUI::IGraphicsItem *>(plugin);
        if (itemObject) {
            if (aReset) {
                itemObject->reset(QVariantMap());
            }

            QWidget *widget = itemObject->getNativeWidget();
            if (widget) {
                widgetList << widget;
            }
        }
    }

    return widgetList;
}

//--------------------------------------------------------------------------
void HumoServiceBackend::startHeartbeat() {
    if (!mHeartbeatTimer.isActive()) {
        mHeartbeatTimer.start();
    }
}

//--------------------------------------------------------------------------
void HumoServiceBackend::stopHeartbeat() {
    if (mHeartbeatTimer.isActive()) {
        mHeartbeatTimer.stop();
    }
}

//--------------------------------------------------------------------------
void HumoServiceBackend::sendHeartbeat() {
    // TODO: Implement heartbeat sending
}

//--------------------------------------------------------------------------