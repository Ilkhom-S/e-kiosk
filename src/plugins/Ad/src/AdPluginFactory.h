/* @file Фабрика плагина рекламы. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/PluginFactory.h>

class AdPluginFactory : public SDK::Plugin::PluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)

  public:
    AdPluginFactory();
};