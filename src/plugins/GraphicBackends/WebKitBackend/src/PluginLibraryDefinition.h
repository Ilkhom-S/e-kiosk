/* @file Определение WebKit графического бэкенда. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

class WebKitBackendPluginFactory : public SDK::Plugin::PluginFactory
{
	Q_OBJECT
	Q_INTERFACES(SDK::Plugin::IPluginFactory)
	Q_PLUGIN_METADATA(IID "Humo.Graphics.Backend.WebKit")
};

//------------------------------------------------------------------------------
