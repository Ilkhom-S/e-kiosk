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
class WebKitBackendFactory : public QObject, public SDK::Plugin::IPluginFactory {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory" FILE "webkit_backend.json")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)

  public:
    // IPluginFactory implementation
    QString getName() const override {
        return "WebKit Graphics Backend";
    }
    QString getDescription() const override {
        return "WebKit based graphics backend for HTML widgets";
    }
    QString getAuthor() const override {
        return "Humo";
    }
    QString getVersion() const override {
        return "1.0";
    }
    QStringList getPluginList() const override {
        return QStringList() << "WebKitBackend.Instance";
    }

    SDK::Plugin::IPlugin *createPlugin(const QString &instancePath) override;
};