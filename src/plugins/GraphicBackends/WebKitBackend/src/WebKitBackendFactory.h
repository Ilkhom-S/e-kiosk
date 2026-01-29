/* @file Фабрика плагина WebKitBackend. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPluginFactory.h>

//------------------------------------------------------------------------------
/// Фабрика плагина WebKitBackend.
class WebKitBackendFactory : public QObject, public SDK::Plugin::IPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
};