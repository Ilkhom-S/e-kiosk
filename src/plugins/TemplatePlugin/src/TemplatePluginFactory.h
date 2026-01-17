#pragma once

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

class TemplatePluginFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory" FILE "template_plugin.json")

  public:
    TemplatePluginFactory();
    ~TemplatePluginFactory();

    // Override base methods if needed for testing
    virtual QString getName() const override;
    virtual QString getDescription() const override;
    virtual QStringList getPluginList() const override;
    virtual SDK::Plugin::IPlugin *createPlugin(const QString &aInstancePath, const QString &aConfigPath) override;
    virtual bool destroyPlugin(SDK::Plugin::IPlugin *aPlugin) override;
};