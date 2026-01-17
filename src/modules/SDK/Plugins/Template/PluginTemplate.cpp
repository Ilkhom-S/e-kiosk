/* @file Шаблон реализации плагина. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

// Project
#include "PluginTemplate.h"

Plugin::Plugin(SDK::Plugin::IEnvironment *aEnvironment, const QString &aInstancePath)
    : mEnvironment(aEnvironment), mInstancePath(aInstancePath), mHelloMessage("Hello World from Plugin Template!") {
    setLog(mEnvironment->getLog("PluginTemplate"));
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
    return mInstancePath;
}

//------------------------------------------------------------------------------
QVariantMap Plugin::getConfiguration() const {
    return mParameters;
}

//------------------------------------------------------------------------------
void Plugin::setConfiguration(const QVariantMap &aParameters) {
    mParameters = aParameters;
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
    return mHelloMessage;
}

//------------------------------------------------------------------------------
