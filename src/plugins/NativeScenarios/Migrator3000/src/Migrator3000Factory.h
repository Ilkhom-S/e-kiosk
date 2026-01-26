#pragma once

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

class Migrator3000Factory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory")

  public:
    Migrator3000Factory();
};
