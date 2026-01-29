/* @file Определение плагина виртуальных устройств. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

class VirtualBillAcceptorPluginFactory : public SDK::Plugin::PluginFactory
{
    Q_OBJECT
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory")

  public:
    /// Конструктор фабрики.
    VirtualBillAcceptorPluginFactory();
};

//------------------------------------------------------------------------------