/* @file Шаблон реализации плагина. */

#include "PluginTemplate.h"

#include <QtCore/QDebug>

Plugin::Plugin(SDK::Plugin::IEnvironment *aEnvironment, const QString &aInstancePath)
    : m_Environment(aEnvironment), m_InstancePath(aInstancePath),
      m_HelloMessage("Hello World from Plugin Template!") {
    setLog(m_Environment->getLog("PluginTemplate"));
    toLog(LogLevel::Normal, "Plugin template initialized");
    // TODO - initialize plugin
}

//------------------------------------------------------------------------------
Plugin::~Plugin() {
    toLog(LogLevel::Normal, "Plugin template destroyed");
}

//------------------------------------------------------------------------------
QString Plugin::getPluginName() const {
    return "Plugin Template";
}

//------------------------------------------------------------------------------
QString Plugin::getConfigurationName() const {
    return m_InstancePath;
}

//------------------------------------------------------------------------------
QVariantMap Plugin::getConfiguration() const {
    return m_Parameters;
}

//------------------------------------------------------------------------------
void Plugin::setConfiguration(const QVariantMap &aParameters) {
    m_Parameters = aParameters;
    toLog(LogLevel::Normal, QString("Configuration updated: %1").arg(aParameters.size()));
}

//------------------------------------------------------------------------------
bool Plugin::saveConfiguration() {
    // В реальном плагине здесь сохраняем в постоянное хранилище
    toLog(LogLevel::Normal, "Configuration saved");
    return true;
}

//------------------------------------------------------------------------------
bool Plugin::isReady() const {
    return true;
}

//------------------------------------------------------------------------------
QString Plugin::getHelloMessage() const {
    return m_HelloMessage;
}

//------------------------------------------------------------------------------
