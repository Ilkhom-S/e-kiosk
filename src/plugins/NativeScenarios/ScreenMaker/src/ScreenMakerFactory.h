/* @file Фабрика плагина ScreenMaker. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

#include <SDK/Plugins/PluginFactory.h>

//---------------------------------------------------------------------------
/// Фабрика плагина ScreenMaker с метаданными Qt.
class ScreenMakerPluginFactory : public SDK::Plugin::PluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)

  public:
    /// Конструктор фабрики.
    ScreenMakerPluginFactory();
};
