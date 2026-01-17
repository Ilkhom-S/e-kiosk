#pragma once

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

class QMLBackendFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory" FILE "qml_backend.json")

  public:
    QMLBackendFactory();
    ~QMLBackendFactory();

    // Override base methods if needed for QML-specific behavior
    virtual QString getName() const override;
    virtual QString getDescription() const override;
};