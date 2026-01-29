/* @file Определение виджета меню обслуживания. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

class ServiceMenuPluginFactory : public SDK::Plugin::PluginFactory
{
    Q_OBJECT
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
    Q_PLUGIN_METADATA(IID "com.humo.service_menu")
};

//------------------------------------------------------------------------------
