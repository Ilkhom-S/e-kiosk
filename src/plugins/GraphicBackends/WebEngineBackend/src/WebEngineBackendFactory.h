/* @file Фабрика плагина WebEngineBackend. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPluginFactory.h>

//------------------------------------------------------------------------------
/// Фабрика плагина WebEngineBackend.
class WebEngineBackendFactory : public QObject, public SDK::Plugin::IPluginFactory {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory" FILE "webengine_backend.json")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)

  public:
    // IPluginFactory implementation
    QString getName() const override {
        return "WebEngine Graphics Backend";
    }
    QString getDescription() const override {
        return "WebEngine based graphics backend for HTML widgets";
    }
    QString getAuthor() const override {
        return "Humo";
    }
    QString getVersion() const override {
        return "1.0";
    }
    QStringList getPluginList() const override {
        return QStringList() << "WebEngineBackend.Instance";
    }

    /// Создаёт экземпляр плагина WebEngineBackend.
    SDK::Plugin::IPlugin *createPlugin(const QString &instancePath) override;
};