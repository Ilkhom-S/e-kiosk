/* @file Фабрика плагина ScreenMaker. */

#pragma once

#include <QtCore/QObject>

#include <SDK/Plugins/PluginFactory.h>

//---------------------------------------------------------------------------
/// Фабрика плагина ScreenMaker с метаданными Qt.
class ScreenMakerPluginFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)

public:
    /// Конструктор фабрики.
    ScreenMakerPluginFactory();
};
