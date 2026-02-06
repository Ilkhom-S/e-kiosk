/* @file Определение плагина принтера. */

// Plugin SDK

#include <SDK/Plugins/PluginFactory.h>

class PrintersPluginFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
    Q_PLUGIN_METADATA(IID "com.humo.printers")

public:
    /// Конструктор фабрики.
    PrintersPluginFactory();
};

//------------------------------------------------------------------------------
