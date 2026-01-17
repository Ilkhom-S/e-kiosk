// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPluginEnvironment.h>

// Project
#include "TemplatePlugin.h"

TemplatePlugin::TemplatePlugin(SDK::Plugin::IEnvironment *aEnvironment, const QString &aInstancePath)
    : mEnvironment(aEnvironment), mInstancePath(aInstancePath), mHelloMessage("Hello from Template Plugin!") {
    qDebug() << "TemplatePlugin created with instance path:" << aInstancePath;
}

TemplatePlugin::~TemplatePlugin() {
    qDebug() << "TemplatePlugin destroyed";
}

QString TemplatePlugin::getPluginName() const {
    return "Template Plugin";
}

QString TemplatePlugin::getConfigurationName() const {
    return mInstancePath;
}

QVariantMap TemplatePlugin::getConfiguration() const {
    return mConfiguration;
}

void TemplatePlugin::setConfiguration(const QVariantMap &aConfiguration) {
    mConfiguration = aConfiguration;
    qDebug() << "TemplatePlugin configuration set:" << aConfiguration;
}

bool TemplatePlugin::saveConfiguration() {
    // In a real plugin, this would save to persistent storage
    // For template, just return true
    qDebug() << "TemplatePlugin saveConfiguration called";
    return true;
}

bool TemplatePlugin::isReady() const {
    return true;
}

QString TemplatePlugin::getHelloMessage() const {
    return mHelloMessage;
}