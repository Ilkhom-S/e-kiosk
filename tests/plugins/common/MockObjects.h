// Plugin Testing Framework - Mock implementations for isolated plugin testing

#pragma once

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>

// Plugin SDK
#include <Common/ILog.h>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/Plugins/IKernel.h>
#include <SDK/Plugins/IPluginLoader.h>

// Mock logger for testing
class MockLog : public ILog {
public:
    MockLog(const QString &name = "MockLog") : m_Name(name), m_Level(LogLevel::Normal) {}

    // ILog interface
    virtual const QString &getName() const override { return m_Name; }
    virtual LogType::Enum getType() const override { return LogType::Debug; }
    virtual const QString &getDestination() const override { return m_Destination; }
    virtual void setDestination(const QString &aDestination) override {
        m_Destination = aDestination;
    }
    virtual void setLevel(LogLevel::Enum aLevel) override { m_Level = aLevel; }
    virtual void adjustPadding(int aStep) override { /* Mock implementation - do nothing */ }
    virtual void write(LogLevel::Enum aLevel, const QString &aMessage) override {
        // Don't log during Qt shutdown to avoid crashes
        if (QCoreApplication::instance() == nullptr) {
            return;
        }
        if (aLevel >= m_Level) {
            m_Messages.append(QString("[%1] %2").arg(logLevelToString(aLevel)).arg(aMessage));
        }
    }
    virtual void
    write(LogLevel::Enum aLevel, const QString &aMessage, const QByteArray &aData) override {
        write(aLevel, aMessage + " [binary data: " + QString::number(aData.size()) + " bytes]");
    }
    virtual void logRotate() override {}
    virtual ~MockLog() {}

    // Static methods
    static ILog *getInstance() { return new MockLog(); }
    static ILog *getInstance(const QString &aName) { return new MockLog(aName); }
    static ILog *getInstance(const QString &aName, LogType::Enum aType) {
        return new MockLog(aName);
    }
    static void logRotateAll() {}
    static void setGlobalLevel(LogLevel::Enum) {}

    // Test helpers
    QStringList getMessages() const { return m_Messages; }
    void clearMessages() { m_Messages.clear(); }

private:
    QString logLevelToString(LogLevel::Enum level) const {
        switch (level) {
        case LogLevel::Off:
            return "OFF";
        case LogLevel::Fatal:
            return "FATAL";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Warning:
            return "WARN";
        case LogLevel::Normal:
            return "INFO";
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Trace:
            return "TRACE";
        default:
            return "UNKNOWN";
        }
    }

    QString m_Name;
    QString m_Destination;
    LogLevel::Enum m_Level;
    QStringList m_Messages;
};

// Mock plugin loader for testing
class MockPluginLoader : public SDK::Plugin::IPluginLoader {
public:
    MockPluginLoader() {}

    // IPluginLoader interface
    virtual int addDirectory(const QString &aDirectory) override {
        m_Directories.append(aDirectory);
        return 1; // Simulate success
    }

    virtual QStringList getPluginList(const QRegularExpression &aFilter) const override {
        QStringList result;
        foreach (QString plugin, m_RegisteredPlugins) {
            if (aFilter.pattern().isEmpty() || aFilter.match(plugin).capturedStart() != -1) {
                result.append(plugin);
            }
        }
        return result;
    }

    virtual QStringList getPluginPathList(const QRegularExpression &aFilter) const override {
        return getPluginList(aFilter); // Simplified
    }

    virtual SDK::Plugin::IPlugin *createPlugin(const QString &aInstancePath,
                                               const QString &aConfigPath = "") override {
        return nullptr; // Not implemented for basic testing
    }

    virtual std::weak_ptr<SDK::Plugin::IPlugin>
    createPluginPtr(const QString &aInstancePath, const QString &aConfigPath = "") override {
        return std::weak_ptr<SDK::Plugin::IPlugin>();
    }

    virtual QVariantMap getPluginInstanceConfiguration(const QString &aInstancePath,
                                                       const QString &aConfigPath) override {
        return QVariantMap();
    }

    virtual SDK::Plugin::TParameterList
    getPluginParametersDescription(const QString &aPath) const override {
        return SDK::Plugin::TParameterList();
    }

    virtual bool destroyPlugin(SDK::Plugin::IPlugin *aPlugin) override {
        return true; // Mock implementation - always succeed
    }

    virtual bool destroyPluginPtr(const std::weak_ptr<SDK::Plugin::IPlugin> &aPlugin) override {
        return true; // Mock implementation - always succeed
    }

    // Test helpers
    void registerPlugin(const QString &pluginPath) { m_RegisteredPlugins.append(pluginPath); }

private:
    QStringList m_Directories;
    QStringList m_RegisteredPlugins;
};

class MockCore : public SDK::PaymentProcessor::ICore, public SDK::Plugin::IExternalInterface {
public:
    SDK::PaymentProcessor::IRemoteService *getRemoteService() const override { return nullptr; }
    SDK::PaymentProcessor::IPaymentService *getPaymentService() const override { return nullptr; }
    SDK::PaymentProcessor::IFundsService *getFundsService() const override { return nullptr; }
    SDK::PaymentProcessor::IPrinterService *getPrinterService() const override { return nullptr; }
    SDK::PaymentProcessor::IHIDService *getHIDService() const override { return nullptr; }
    SDK::PaymentProcessor::INetworkService *getNetworkService() const override { return nullptr; }
    SDK::PaymentProcessor::IEventService *getEventService() const override { return nullptr; }
    SDK::PaymentProcessor::IGUIService *getGUIService() const override { return nullptr; }
    SDK::PaymentProcessor::IDeviceService *getDeviceService() const override { return nullptr; }
    SDK::PaymentProcessor::ICryptService *getCryptService() const override { return nullptr; }
    SDK::PaymentProcessor::ISettingsService *getSettingsService() const override { return nullptr; }
    SDK::PaymentProcessor::IDatabaseService *getDatabaseService() const override { return nullptr; }
    SDK::PaymentProcessor::ITerminalService *getTerminalService() const override { return nullptr; }
    SDK::PaymentProcessor::ISchedulerService *getSchedulerService() const override {
        return nullptr;
    }
    QSet<SDK::PaymentProcessor::IService *> getServices() const override { return {}; }
    SDK::PaymentProcessor::IService *getService(const QString &) const override { return nullptr; }
    QVariantMap &getUserProperties() override { return m_UserProperties; }
    bool canShutdown() override { return true; }

private:
    QVariantMap m_UserProperties;
};

// Mock kernel for plugin testing
class MockKernel : public SDK::Plugin::IKernel {
public:
    MockKernel() : m_PluginLoader(new MockPluginLoader()) {}

    ~MockKernel() { delete m_PluginLoader; }

    // IKernel interface
    virtual ILog *getLog(const QString &aName = "") const override {
        return new MockLog(aName.isEmpty() ? "MockKernel" : aName);
    }

    virtual QString getVersion() const override { return "1.0.0-test"; }

    virtual QString getDirectory() const override { return QDir::currentPath(); }

    virtual QString getDataDirectory() const override { return QDir::currentPath() + "/data"; }

    virtual QString getLogsDirectory() const override { return QDir::currentPath() + "/logs"; }

    virtual bool canConfigurePlugin(const QString &aInstancePath) const override { return true; }

    virtual QVariantMap getPluginConfiguration(const QString &aInstancePath) const override {
        return m_Configurations.value(aInstancePath, QVariantMap());
    }

    virtual bool canSavePluginConfiguration(const QString &aInstancePath) const override {
        return true;
    }

    virtual bool savePluginConfiguration(const QString &aInstancePath,
                                         const QVariantMap &aParameters) override {
        m_Configurations[aInstancePath] = aParameters;
        return true;
    }

    virtual SDK::Plugin::IExternalInterface *getInterface(const QString &aInterface) override {
        if (aInterface == SDK::PaymentProcessor::CInterfaces::ICore) {
            return &m_Core;
        }

        return nullptr;
    }

    virtual SDK::Plugin::IPluginLoader *getPluginLoader() const override { return m_PluginLoader; }

    // Test helpers
    void setPluginConfiguration(const QString &instancePath, const QVariantMap &config) {
        m_Configurations[instancePath] = config;
    }

    MockPluginLoader *getMockPluginLoader() const { return m_PluginLoader; }

private:
    MockPluginLoader *m_PluginLoader;
    QMap<QString, QVariantMap> m_Configurations;
    MockCore m_Core;
};
