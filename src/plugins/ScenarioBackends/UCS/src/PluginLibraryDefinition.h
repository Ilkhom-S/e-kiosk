/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include <SDK/Plugins/PluginFactory.h>

class UcsPluginFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
    Q_PLUGIN_METADATA(IID "com.humo.ucs")

public:
    /// Конструктор фабрики.
    UcsPluginFactory();
};

//------------------------------------------------------------------------------
