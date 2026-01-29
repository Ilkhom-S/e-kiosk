/* @file Фабрика плагина сторожевых таймеров. */
#pragma once

#include <SDK/Plugins/PluginFactory.h>

class WatchdogPluginFactory : public SDK::Plugin::PluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
  public:
    WatchdogPluginFactory();
    ~WatchdogPluginFactory() override = default;
};
