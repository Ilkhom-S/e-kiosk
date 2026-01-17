/* @file Фабрика плагина рекламы. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/PluginFactory.h>

//---------------------------------------------------------------------------
// Note: Adopted from original TerminalClient Ad plugin implementation
// Platform-specific: Supports both Qt5 and Qt6 for cross-platform compatibility
class AdPluginFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory" FILE "plugin.json")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)

  public:
    //---------------------------------------------------------------------------
    // Конструктор фабрики плагина
    AdPluginFactory();
    ~AdPluginFactory();

    // Override base methods for Ad-specific behavior
    virtual QString getName() const override;
    virtual QString getDescription() const override;
    virtual QString getAuthor() const override;
    virtual QString getVersion() const override;
    virtual QStringList getPluginList() const override;

    // Override shutdown to avoid logging during Qt test cleanup
    virtual void shutdown() override;

    SDK::Plugin::IPlugin *createPlugin(const QString &instancePath, const QString &configPath) override;
};

//---------------------------------------------------------------------------