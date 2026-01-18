/* @file Фабрика плагина WebEngineBackend. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/PluginFactory.h>

//------------------------------------------------------------------------------
/// Фабрика плагина WebEngineBackend.
class WebEngineBackendFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory" FILE "webengine_backend.json")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
};