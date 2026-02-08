// Base class for plugin testing

#pragma once

#include <QtCore/QPluginLoader>
#include <QtTest/QtTest>

// Plugin SDK
#include <SDK/Plugins/IPluginFactory.h>

// Mock objects
#include "MockObjects.h"

class PluginTestBase {

public:
    PluginTestBase(const QString &pluginPath) : m_PluginPath(pluginPath) {}

    // Load plugin and return factory
    SDK::Plugin::IPluginFactory *loadPluginFactory() {
        if (m_Loader.isLoaded()) {
            return m_Factory;
        }

        m_Loader.setFileName(m_PluginPath);
        QObject *rootObject = m_Loader.instance();

        if (!rootObject) {
            qWarning() << "Failed to load plugin:" << m_Loader.errorString();
            return nullptr;
        }

        m_Factory = qobject_cast<SDK::Plugin::IPluginFactory *>(rootObject);
        if (!m_Factory) {
            qWarning() << "Plugin does not implement IPluginFactory";
            return nullptr;
        }

        // Initialize with mock kernel
        if (!m_Factory->initialize(&m_MockKernel, QFileInfo(m_PluginPath).absolutePath())) {
            qWarning() << "Failed to initialize plugin factory";
            return nullptr;
        }

        return m_Factory;
    }

    // Get mock kernel for advanced testing
    MockKernel &getMockKernel() { return m_MockKernel; }

    // Get mock plugin loader
    MockPluginLoader *getMockPluginLoader() { return m_MockKernel.getMockPluginLoader(); }

private:
    QString m_PluginPath;
    QPluginLoader m_Loader;
    SDK::Plugin::IPluginFactory *m_Factory = nullptr;
    MockKernel m_MockKernel;
};