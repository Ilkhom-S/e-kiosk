/* @file Фабрика плагина WebEngineBackend. */

#pragma once

#include <QtCore/QObject>

#include <SDK/Plugins/PluginFactory.h>

//------------------------------------------------------------------------------
/// Фабрика плагина WebEngineBackend.
class WebEngineBackendFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)

public:
    WebEngineBackendFactory();
};