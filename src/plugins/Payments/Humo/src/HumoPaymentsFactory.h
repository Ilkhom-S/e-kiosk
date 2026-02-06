/* @file Фабрика плагина HumoPayments с метаданными Qt. */

#pragma once

#include <QtCore/QObject>

#include <SDK/Plugins/PluginFactory.h>

//---------------------------------------------------------------------------
/// Фабрика плагина HumoPayments с метаданными Qt.
class HumoPaymentsFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)

public:
    HumoPaymentsFactory();
    ~HumoPaymentsFactory() override = default;
};
