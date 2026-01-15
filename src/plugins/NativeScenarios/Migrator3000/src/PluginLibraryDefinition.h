/* @file Определение сценария мигратора 3000. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

class Migrator3000PluginFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
    Q_PLUGIN_METADATA(IID "com.humo.migrator3000")
};

//------------------------------------------------------------------------------
