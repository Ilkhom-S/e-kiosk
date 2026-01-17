/* @file Фабрика плагина нативного бэкенда. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/PluginFactory.h>

//---------------------------------------------------------------------------
// Note: Adopted from original TerminalClient NativeBackend plugin implementation
// Platform-specific: Compatible with Qt5/Qt6 for native widget graphics backend
class NativeBackendFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory" FILE "native_backend.json")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)

  public:
    //---------------------------------------------------------------------------
    // Конструктор фабрики
    NativeBackendFactory();
    ~NativeBackendFactory();

    // Override base methods for NativeBackend-specific behavior
    virtual QString getName() const override;
    virtual QString getDescription() const override;
    virtual QString getAuthor() const override;
    virtual QString getVersion() const override;
    virtual QStringList getPluginList() const override;

    SDK::Plugin::IPlugin *createPlugin(const QString &instancePath, const QString &configPath) override;
};

//---------------------------------------------------------------------------