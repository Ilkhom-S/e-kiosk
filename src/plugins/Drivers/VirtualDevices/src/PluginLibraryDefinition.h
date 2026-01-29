/* @file Определение плагина виртуальных устройств. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

class VirtualBillAcceptorPluginFactory : public SDK::Plugin::PluginFactory
{
    Q_OBJECT
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
    Q_PLUGIN_METADATA(IID "com.humo.virtual_bill_acceptor")
};

//------------------------------------------------------------------------------