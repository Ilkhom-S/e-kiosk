/* @file Плагин - источник рекламы. */

#include "AdSourcePlugin.h"

#include <QtCore/QCoreApplication>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/PluginInitializer.h>

#include <AdBackend/Client.h>

#include "AdRemotePlugin.h"

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
namespace CAdRemotePlugin {
const char PluginName[] = "HumoAd";
} // namespace CAdRemotePlugin

//------------------------------------------------------------------------------
namespace {
/// Конструктор экземпляра плагина.
SDK::Plugin::IPlugin *CreateAdSourcePlugin(SDK::Plugin::IEnvironment *aFactory,
                                           const QString &aInstancePath) {
    return new AdSourcePlugin(aFactory, aInstancePath);
}

/// Регистрация плагина в фабрике.
REGISTER_PLUGIN(makePath(PPSDK::Application,
                         PPSDK::CComponents::AdSource,
                         CAdRemotePlugin::PluginName),
                &CreateAdSourcePlugin,
                &SDK::Plugin::PluginInitializer::emptyParameterList,
                AdSourcePlugin);
} // namespace

//---------------------------------------------------------------------------
// Конструктор плагина источника рекламы
AdSourcePlugin::AdSourcePlugin(SDK::Plugin::IEnvironment *aFactory, const QString &aInstancePath)
    : m_Factory(aFactory), m_InstancePath(aInstancePath) {
    m_Client = getAdClientInstance(aFactory);

    m_Core = dynamic_cast<SDK::PaymentProcessor::ICore *>(
        m_Factory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
}

//------------------------------------------------------------------------------
AdSourcePlugin::~AdSourcePlugin(void) {
    m_Client.clear();
}

//------------------------------------------------------------------------------
QString AdSourcePlugin::getPluginName() const {
    return CAdRemotePlugin::PluginName;
}

//------------------------------------------------------------------------------
QVariantMap AdSourcePlugin::getConfiguration() const {
    return QVariantMap();
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
