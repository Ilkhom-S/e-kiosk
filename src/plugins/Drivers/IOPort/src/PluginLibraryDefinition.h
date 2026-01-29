/* @file Определение плагина порта ввода-вывода. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

class IOPortsPluginFactory : public SDK::Plugin::PluginFactory
{
    Q_OBJECT
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
    Q_PLUGIN_METADATA(IID "com.humo.ioports")

  public:
    /// Конструктор фабрики.
    IOPortsPluginFactory();
};

//------------------------------------------------------------------------------
