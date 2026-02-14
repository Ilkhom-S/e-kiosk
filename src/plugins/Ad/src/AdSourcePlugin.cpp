/* @file Плагин - источник рекламы. */

#include "AdSourcePlugin.h"

#include <QtCore/QCoreApplication>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/PluginInitializer.h>

#include <AdBackend/Client.h>
#include <utility>

#include "AdRemotePlugin.h"

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
namespace CAdRemotePlugin {
const char PluginName[] = "HumoAd";
} // namespace CAdRemotePlugin

//------------------------------------------------------------------------------
namespace {
/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin *createAdSourcePlugin(SDK::Plugin::IEnvironment *aFactory,
                                           const QString &aInstancePath) {
    return new AdSourcePlugin(aFactory, aInstancePath);
}

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN(makePath(PPSDK::Application,
                         PPSDK::CComponents::AdSource,
                         CAdRemotePlugin::PluginName),
                &createAdSourcePlugin,
                &SDK::Plugin::PluginInitializer::emptyParameterList,
                AdSourcePlugin);
} // namespace

//---------------------------------------------------------------------------
// Конструктор плагина источника рекламы
AdSourcePlugin::AdSourcePlugin(SDK::Plugin::IEnvironment *aFactory, QString aInstancePath)
    : m_Factory(aFactory), m_InstancePath(std::move(aInstancePath)) {
    m_Client = getAdClientInstance(aFactory);

    void *voidPtr = reinterpret_cast<void *>(
        m_Factory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
    m_Core = reinterpret_cast<SDK::PaymentProcessor::ICore *>(voidPtr);
}

//------------------------------------------------------------------------------
AdSourcePlugin::~AdSourcePlugin() {
    m_Client.clear();
}

//------------------------------------------------------------------------------
QString AdSourcePlugin::getPluginName() const {
    return CAdRemotePlugin::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap AdSourcePlugin::getConfiguration() const {
    return {};
}

//------------------------------------------------------------------------------
void AdSourcePlugin::setConfiguration(const QVariantMap &aParameters) {
    Q_UNUSED(aParameters)
}

//------------------------------------------------------------------------------
QString AdSourcePlugin::getConfigurationName() const {
    return m_InstancePath;
}

//------------------------------------------------------------------------------
bool AdSourcePlugin::saveConfiguration() {
    // У плагина нет параметров
    return true;
}

//------------------------------------------------------------------------------
bool AdSourcePlugin::isReady() const {
    return true;
}

//------------------------------------------------------------------------------
QString AdSourcePlugin::getContent(const QString &aType) const {
    return m_Client->getContent(aType);
}

//------------------------------------------------------------------------------
void AdSourcePlugin::addEvent(const QString &aType, const QVariantMap &aParameters) {
    Q_UNUSED(aParameters)

    // TODO use aParameters!!!
    m_Client->addEvent(aType);
}

//------------------------------------------------------------------------------
