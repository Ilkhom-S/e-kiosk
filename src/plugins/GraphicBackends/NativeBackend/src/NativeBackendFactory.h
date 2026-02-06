/* @file Объявление фабрики плагина NativeBackend. */

#pragma once

#include <QtCore/QObject>

#include <SDK/Plugins/PluginFactory.h>

#include "NativeBackend.h"

class NativeBackendFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)

public:
    NativeBackendFactory();
};