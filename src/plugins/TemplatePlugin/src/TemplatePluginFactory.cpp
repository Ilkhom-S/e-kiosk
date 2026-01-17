// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "TemplatePlugin.h"
#include "TemplatePluginFactory.h"

QString SDK::Plugin::PluginFactory::mName = "Template Plugin";
QString SDK::Plugin::PluginFactory::mDescription = "Minimal template plugin for plugin development";
QString SDK::Plugin::PluginFactory::mAuthor = "EKiosk Template";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName = "template_plugin";

TemplatePluginFactory::TemplatePluginFactory() {
    qDebug() << "TemplatePluginFactory created";
}

TemplatePluginFactory::~TemplatePluginFactory() {
    qDebug() << "TemplatePluginFactory destroyed";
}

QString TemplatePluginFactory::getName() const {
    return "Template Plugin (Overridden)";
}

QString TemplatePluginFactory::getDescription() const {
    return "Minimal template plugin for plugin development (Overridden)";
}

QStringList TemplatePluginFactory::getPluginList() const {
    return QStringList() << "TemplatePlugin.Instance";
}

SDK::Plugin::IPlugin *TemplatePluginFactory::createPlugin(const QString &aInstancePath, const QString &aConfigPath) {
    qDebug() << "Creating TemplatePlugin with instance path:" << aInstancePath;
    TemplatePlugin *plugin = new TemplatePlugin(this, aInstancePath);
    mCreatedPlugins[plugin] = aInstancePath; // Track the plugin for proper destruction
    return plugin;
}

bool TemplatePluginFactory::destroyPlugin(SDK::Plugin::IPlugin *aPlugin) {
    if (aPlugin && mCreatedPlugins.contains(aPlugin)) {
        qDebug() << "Destroying TemplatePlugin:" << aPlugin->getPluginName();
        mCreatedPlugins.remove(aPlugin);
        delete static_cast<TemplatePlugin *>(aPlugin);
        return true;
    }
    return false;
}