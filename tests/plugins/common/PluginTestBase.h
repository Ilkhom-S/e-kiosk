// Base class for plugin testing

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtTest/QtTest>
#include <QtCore/QPluginLoader>
#include <Common/QtHeadersEnd.h>

// Plugin SDK
#include <SDK/Plugins/IPluginFactory.h>

// Mock objects
#include "MockObjects.h"

class PluginTestBase {

  public:
    PluginTestBase(const QString &pluginPath) : mPluginPath(pluginPath) {
    }

    // Load plugin and return factory
    SDK::Plugin::IPluginFactory *loadPluginFactory() {
        if (mLoader.isLoaded()) {
            return mFactory;
        }

        mLoader.setFileName(mPluginPath);
        QObject *rootObject = mLoader.instance();

        if (!rootObject) {
            qWarning() << "Failed to load plugin:" << mLoader.errorString();
            return nullptr;
        }

        mFactory = qobject_cast<SDK::Plugin::IPluginFactory *>(rootObject);
        if (!mFactory) {
            qWarning() << "Plugin does not implement IPluginFactory";
            return nullptr;
        }

        // Initialize with mock kernel
        if (!mFactory->initialize(&mMockKernel, QFileInfo(mPluginPath).absolutePath())) {
            qWarning() << "Failed to initialize plugin factory";
            return nullptr;
        }

        return mFactory;
    }

    // Get mock kernel for advanced testing
    MockKernel &getMockKernel() {
        return mMockKernel;
    }

    // Get mock plugin loader
    MockPluginLoader *getMockPluginLoader() {
        return mMockKernel.getMockPluginLoader();
    }

  private:
    QString mPluginPath;
    QPluginLoader mLoader;
    SDK::Plugin::IPluginFactory *mFactory = nullptr;
    MockKernel mMockKernel;
};