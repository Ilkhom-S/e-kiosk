/* @file Определение плагина монетоприемника. */

// Plugin SDK

#include <SDK/Plugins/PluginFactory.h>

class CoinAcceptorPluginFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
    Q_PLUGIN_METADATA(IID "com.humo.coin_acceptors")

public:
    /// Конструктор фабрики.
    CoinAcceptorPluginFactory();
};

//------------------------------------------------------------------------------
