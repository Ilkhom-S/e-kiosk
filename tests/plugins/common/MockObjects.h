// Plugin Testing Framework - Mock implementations for isolated plugin testing

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariantMap>
#include <QtCore/QDir>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/IKernel.h>
#include <SDK/Plugins/IPluginLoader.h>
#include <Common/ILog.h>

// Mock logger for testing
class MockLog : public ILog {
  public:
    MockLog(const QString &name = "MockLog") : mName(name), mLevel(LogLevel::Normal) {
    }

    // ILog interface
    virtual const QString &getName() const override {
        return mName;
    }
    virtual LogType::Enum getType() const override {
        return LogType::Debug;
    }
    virtual const QString &getDestination() const override {
        return mDestination;
    }
    virtual void setDestination(const QString &aDestination) override {
        mDestination = aDestination;
    }
    virtual void setLevel(LogLevel::Enum aLevel) override {
        mLevel = aLevel;
    }
    virtual void adjustPadding(int aStep) override { /* Mock implementation - do nothing */
    }
    virtual void write(LogLevel::Enum aLevel, const QString &aMessage) override {
        if (aLevel >= mLevel) {
            mMessages.append(QString("[%1] %2").arg(logLevelToString(aLevel)).arg(aMessage));
        }
    }
    virtual void write(LogLevel::Enum aLevel, const QString &aMessage, const QByteArray &aData) override {
        write(aLevel, aMessage + " [binary data: " + QString::number(aData.size()) + " bytes]");
    }
    virtual void logRotate() override {
    }
    virtual ~MockLog() {
    }

    // Static methods
    static ILog *getInstance() {
        return new MockLog();
    }
    static ILog *getInstance(const QString &aName) {
        return new MockLog(aName);
    }
    static ILog *getInstance(const QString &aName, LogType::Enum aType) {
        return new MockLog(aName);
    }
    static void logRotateAll() {
    }
    static void setGlobalLevel(LogLevel::Enum) {
    }

    // Test helpers
    QStringList getMessages() const {
        return mMessages;
    }
    void clearMessages() {
        mMessages.clear();
    }

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

    QString mName;
    QString mDestination;
    LogLevel::Enum mLevel;
    QStringList mMessages;
};

// Mock plugin loader for testing
class MockPluginLoader : public SDK::Plugin::IPluginLoader {
  public:
    MockPluginLoader() {
    }

    // IPluginLoader interface
    virtual int addDirectory(const QString &aDirectory) override {
        mDirectories.append(aDirectory);
        return 1; // Simulate success
    }

    virtual QStringList getPluginList(const QRegExp &aFilter) const override {
        QStringList result;
        foreach (QString plugin, mRegisteredPlugins) {
            if (aFilter.isEmpty() || aFilter.indexIn(plugin) != -1) {
                result.append(plugin);
            }
        }
        return result;
    }

    virtual QStringList getPluginPathList(const QRegExp &aFilter) const override {
        return getPluginList(aFilter); // Simplified
    }

    virtual SDK::Plugin::IPlugin *createPlugin(const QString &aInstancePath, const QString &aConfigPath = "") override {
        return nullptr; // Not implemented for basic testing
    }

    virtual std::weak_ptr<SDK::Plugin::IPlugin> createPluginPtr(const QString &aInstancePath,
                                                                const QString &aConfigPath = "") override {
        return std::weak_ptr<SDK::Plugin::IPlugin>();
    }

    virtual QVariantMap getPluginInstanceConfiguration(const QString &aInstancePath,
                                                       const QString &aConfigPath) override {
        return QVariantMap();
    }

    virtual SDK::Plugin::TParameterList getPluginParametersDescription(const QString &aPath) const override {
        return SDK::Plugin::TParameterList();
    }

    virtual bool destroyPlugin(SDK::Plugin::IPlugin *aPlugin) override {
        return true; // Mock implementation - always succeed
    }

    virtual bool destroyPluginPtr(const std::weak_ptr<SDK::Plugin::IPlugin> &aPlugin) override {
        return true; // Mock implementation - always succeed
    }

    // Test helpers
    void registerPlugin(const QString &pluginPath) {
        mRegisteredPlugins.append(pluginPath);
    }

  private:
    QStringList mDirectories;
    QStringList mRegisteredPlugins;
};

// Mock kernel for plugin testing
class MockKernel : public SDK::Plugin::IKernel {
  public:
    MockKernel() : mPluginLoader(new MockPluginLoader()) {
    }

    ~MockKernel() {
        delete mPluginLoader;
    }

    // IKernel interface
    virtual ILog *getLog(const QString &aName = "") const override {
        return new MockLog(aName.isEmpty() ? "MockKernel" : aName);
    }

    virtual QString getVersion() const override {
        return "1.0.0-test";
    }

    virtual QString getDirectory() const override {
        return QDir::currentPath();
    }

    virtual QString getDataDirectory() const override {
        return QDir::currentPath() + "/data";
    }

    virtual QString getLogsDirectory() const override {
        return QDir::currentPath() + "/logs";
    }

    virtual bool canConfigurePlugin(const QString &aInstancePath) const override {
        return true;
    }

    virtual QVariantMap getPluginConfiguration(const QString &aInstancePath) const override {
        return mConfigurations.value(aInstancePath, QVariantMap());
    }

    virtual bool canSavePluginConfiguration(const QString &aInstancePath) const override {
        return true;
    }

    virtual bool savePluginConfiguration(const QString &aInstancePath, const QVariantMap &aParameters) override {
        mConfigurations[aInstancePath] = aParameters;
        return true;
    }

    virtual SDK::Plugin::IExternalInterface *getInterface(const QString &aInterface) override {
        return nullptr; // Not implemented for basic testing
    }

    virtual SDK::Plugin::IPluginLoader *getPluginLoader() const override {
        return mPluginLoader;
    }

    // Test helpers
    void setPluginConfiguration(const QString &instancePath, const QVariantMap &config) {
        mConfigurations[instancePath] = config;
    }

    MockPluginLoader *getMockPluginLoader() const {
        return mPluginLoader;
    }

  private:
    MockPluginLoader *mPluginLoader;
    QMap<QString, QVariantMap> mConfigurations;
};