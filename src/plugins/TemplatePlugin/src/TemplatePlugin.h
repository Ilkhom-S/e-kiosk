#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPlugin.h>

class Q_DECL_EXPORT TemplatePlugin : public QObject, public SDK::Plugin::IPlugin {
    Q_OBJECT

  public:
    TemplatePlugin(SDK::Plugin::IEnvironment *aEnvironment, const QString &aInstancePath);
    ~TemplatePlugin();

    // IPlugin interface
    QString getPluginName() const override;
    QString getConfigurationName() const override;
    QVariantMap getConfiguration() const override;
    void setConfiguration(const QVariantMap &aConfiguration) override;
    bool saveConfiguration() override;
    bool isReady() const override;

    // Template functionality
    QString getHelloMessage() const;

  private:
    SDK::Plugin::IEnvironment *mEnvironment;
    QString mInstancePath;
    QVariantMap mConfiguration;
    QString mHelloMessage;
};