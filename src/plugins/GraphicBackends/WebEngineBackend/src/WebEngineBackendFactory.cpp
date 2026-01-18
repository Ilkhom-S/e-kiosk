/* @file Реализация фабрики плагина WebEngineBackend. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPlugin.h>

// Project
#include "WebEngineBackend.h"
#include "WebEngineBackendFactory.h"

SDK::Plugin::IPlugin *WebEngineBackendFactory::createPlugin(const QString &instancePath, const QString &configPath) {
    return new WebEngineBackend(this, instancePath);
}