/* @file Фабрика плагинов TemplatePlugin. */

#pragma once

// Plugin SDK
#include <SDK/Plugins/PluginFactory.h>

/// Фабрика для создания экземпляров TemplatePlugin.
/// Отвечает за жизненный цикл плагинов TemplatePlugin.
class TemplatePluginFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory")

public:
    /// Конструктор фабрики.
    TemplatePluginFactory();

    /// Деструктор фабрики.
    ~TemplatePluginFactory();
};