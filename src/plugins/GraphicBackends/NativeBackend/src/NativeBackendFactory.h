/* @file Объявление фабрики плагина NativeBackend. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "NativeBackend.h"

class NativeBackendFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)

  public:
    NativeBackendFactory();
};