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
    PluginTestBase() = default;
    PluginTestBase(const QString &pluginPath) : m_PluginPath(pluginPath) {}

    // Helper to resolve plugin path by base name from build output
    static QString findPlugin(const QString &requestedBase) {
        QDir startDir(QCoreApplication::applicationDirPath());
        startDir.cdUp();

        qDebug() << "PluginTestBase::findPlugin startDir=" << startDir.absolutePath();

        QStringList candidates = {startDir.filePath("bin/plugins"),
                                  startDir.filePath("bin/plugins/backends"),
                                  startDir.filePath("bin/plugins/NativeScenarios"),
                                  startDir.filePath("bin/plugins/Payments"),
                                  startDir.filePath("bin/plugins/drivers")};

        for (const QString &cand : candidates) {
            QDir dir(cand);
            qDebug() << "  checking candidate:" << cand << "exists=" << dir.exists();
            if (!dir.exists())
                continue;
            QDirIterator it(dir.absolutePath(), QDirIterator::Subdirectories);
            while (it.hasNext()) {
                it.next();
                QString fn = it.fileName();
                if (fn.contains(requestedBase, Qt::CaseInsensitive)) {
                    qDebug() << "    matched:" << it.filePath();
                    return it.filePath();
                }
            }
        }

        // Fallback: search build dir recursively
        QDirIterator it(startDir.absolutePath(), QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            QString fn = it.fileName();
            if (fn.contains(requestedBase, Qt::CaseInsensitive) &&
                (fn.endsWith('.' + QStringLiteral("dylib")) ||
                 fn.endsWith('.' + QStringLiteral("so")) ||
                 fn.endsWith('.' + QStringLiteral("dll")))) {
                return it.filePath();
            }
        }

        return {};
    }

    // Load plugin and return factory
    SDK::Plugin::IPluginFactory *loadPluginFactory() {
        if (m_Loader.isLoaded()) {
            return m_Factory;
        }

        // If provided path doesn't exist, try to locate plugin in build output
        if (!QFile::exists(m_PluginPath)) {
            QString requestedBase = QFileInfo(m_PluginPath).baseName();
            qDebug() << "Plugin not found at provided path; searching for" << requestedBase;

            // Start search from test executable directory's parent (build dir)
            QDir startDir(QCoreApplication::applicationDirPath());
            startDir.cdUp();

            // Common plugin directories to check first
            QStringList candidates = {startDir.filePath("bin/plugins"),
                                      startDir.filePath("bin/plugins/backends"),
                                      startDir.filePath("bin/plugins/NativeScenarios"),
                                      startDir.filePath("bin/plugins/Payments"),
                                      startDir.filePath("bin/plugins/drivers")};

            QString foundPath;
            for (const QString &cand : candidates) {
                QDir dir(cand);
                if (!dir.exists())
                    continue;
                QDirIterator it(dir.absolutePath(), QDirIterator::Subdirectories);
                while (it.hasNext()) {
                    it.next();
                    QString fn = it.fileName();
                    if (fn.contains(requestedBase, Qt::CaseInsensitive)) {
                        foundPath = it.filePath();
                        break;
                    }
                }
                if (!foundPath.isEmpty())
                    break;
            }

            // Fallback: scan build directory recursively (slower)
            if (foundPath.isEmpty()) {
                QDirIterator it(startDir.absolutePath(), QDirIterator::Subdirectories);
                while (it.hasNext()) {
                    it.next();
                    QString fn = it.fileName();
                    if (fn.contains(requestedBase, Qt::CaseInsensitive) &&
                        (fn.endsWith('.' + QStringLiteral("dylib")) ||
                         fn.endsWith('.' + QStringLiteral("so")) ||
                         fn.endsWith('.' + QStringLiteral("dll")))) {
                        foundPath = it.filePath();
                        break;
                    }
                }
            }

            if (!foundPath.isEmpty()) {
                qDebug() << "Resolved plugin path to" << foundPath;
                m_PluginPath = foundPath;
            } else {
                qWarning() << "Unable to locate plugin for" << requestedBase;
            }
        }

        m_Loader.setFileName(m_PluginPath);
        QObject *rootObject = m_Loader.instance();

        if (!rootObject) {
            qWarning() << "Failed to load plugin:" << m_Loader.errorString()
                       << "path:" << m_PluginPath;
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
