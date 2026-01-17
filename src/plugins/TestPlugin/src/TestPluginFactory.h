#pragma once

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

class TestPluginFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory" FILE "test_plugin.json")

  public:
    TestPluginFactory();
    ~TestPluginFactory();

    // Override base methods if needed for testing
    virtual QString getName() const override;
    virtual QString getDescription() const override;
};