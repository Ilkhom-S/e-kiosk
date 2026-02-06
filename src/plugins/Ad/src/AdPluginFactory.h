/* @file Фабрика плагина рекламы. */

#pragma once

#include <QtCore/QObject>

#include <SDK/Plugins/PluginFactory.h>

class AdPluginFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)

public:
    AdPluginFactory();
};