/* @file Определение виджета меню обслуживания. */

// Plugin SDK

#include <SDK/Plugins/PluginFactory.h>

class ServiceMenuPluginFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
    Q_PLUGIN_METADATA(IID "com.humo.humo_service_menu")

public:
    /// Конструктор фабрики.
    ServiceMenuPluginFactory();
};

//------------------------------------------------------------------------------
