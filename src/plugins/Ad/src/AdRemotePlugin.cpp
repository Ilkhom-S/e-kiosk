/* @file Клиент обновления рекламы. */

#include "AdRemotePlugin.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QMetaObject>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/IRemoteService.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/PluginInitializer.h>

#include <AdBackend/Campaign.h>
#include <AdBackend/Client.h>
#include <AdBackend/DatabaseUtils.h>
#include <utility>

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
namespace CAdRemotePlugin {
const char PluginName[] = "AdRemote";
} // namespace CAdRemotePlugin

//------------------------------------------------------------------------------
namespace {
/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin *createAdSourcePlugin(SDK::Plugin::IEnvironment *aFactory,
                                           const QString &aInstancePath) {
    return new AdRemotePlugin(aFactory, aInstancePath);
}

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN(makePath(PPSDK::Application,
                         PPSDK::CComponents::RemoteClient,
                         CAdRemotePlugin::PluginName),
                &createAdSourcePlugin,
                &SDK::Plugin::PluginInitializer::emptyParameterList,
                AdRemotePlugin);
} // namespace

//------------------------------------------------------------------------
QSharedPointer<Ad::Client> getAdClientInstance(SDK::Plugin::IEnvironment *aFactory) {
    static QSharedPointer<Ad::Client> client;

    if (client.isNull()) {
        void *voidPtr = reinterpret_cast<void *>(aFactory->getInterface(PPSDK::CInterfaces::ICore));
        PPSDK::ICore *core = reinterpret_cast<PPSDK::ICore *>(voidPtr);

        client = QSharedPointer<Ad::Client>(
            new Ad::Client(core, aFactory->getLog(Ad::CClient::LogName), 0));
    }

    return client;
}

//---------------------------------------------------------------------------
// Конструктор плагина
AdRemotePlugin::AdRemotePlugin(SDK::Plugin::IEnvironment *aFactory, QString aInstancePath)
    : ILogable(aFactory->getLog(Ad::CClient::LogName)), m_Factory(aFactory),
      m_InstancePath(std::move(aInstancePath)) {
    m_Client = getAdClientInstance(aFactory);

    void *voidPtr2 = reinterpret_cast<void *>(
        m_Factory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
    m_Core = reinterpret_cast<SDK::PaymentProcessor::ICore *>(voidPtr2);

    connect(m_Client.data(), SIGNAL(contentUpdated()), this, SLOT(needRestart()));
    connect(m_Client.data(), SIGNAL(contentExpired()), this, SLOT(needRestart()));
}

//------------------------------------------------------------------------------
AdRemotePlugin::~AdRemotePlugin() {
    disable();
}

//------------------------------------------------------------------------------
QString AdRemotePlugin::getPluginName() const {
    return CAdRemotePlugin::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap AdRemotePlugin::getConfiguration() const {
    return {};
}

//------------------------------------------------------------------------------
void AdRemotePlugin::setConfiguration(const QVariantMap &aParameters) {
    Q_UNUSED(aParameters)
}

//------------------------------------------------------------------------------
QString AdRemotePlugin::getConfigurationName() const {
    return m_InstancePath;
}

//------------------------------------------------------------------------------
bool AdRemotePlugin::saveConfiguration() {
    // У плагина нет параметров
    return true;
}

//------------------------------------------------------------------------------
bool AdRemotePlugin::isReady() const {
    return true;
}

//------------------------------------------------------------------------------
void AdRemotePlugin::enable() {
    toLog(LogLevel::Normal, "Ad updater plugin enabled.");

    QMetaObject::invokeMethod(m_Client.data(), "reinitialize", Qt::QueuedConnection);
}

//------------------------------------------------------------------------------
void AdRemotePlugin::disable() {
    m_Client.clear();

    toLog(LogLevel::Normal, "Ad updater plugin disabled.");
}

//------------------------------------------------------------------------------
void AdRemotePlugin::needRestart() {
    toLog(LogLevel::Normal, "Send restart command.");

    m_Core->getRemoteService()->registerRestartCommand();
}

//------------------------------------------------------------------------------
SDK::PaymentProcessor::ICore *AdRemotePlugin::getCore() const {
    return m_Core;
}

//------------------------------------------------------------------------------
SDK::PaymentProcessor::IRemoteClient::Capabilities AdRemotePlugin::getCapabilities() const {
    return SDK::PaymentProcessor::IRemoteClient::UpdateContent;
}

//------------------------------------------------------------------------------
bool AdRemotePlugin::useCapability(ECapability aCapability) {
    if (aCapability == IRemoteClient::UpdateContent) {
        QMetaObject::invokeMethod(m_Client.data(), "update", Qt::QueuedConnection);

        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
