/* @file Реализация фабрики плагина WebKitBackend. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPlugin.h>

// Project
#include "WebKitBackend.h"
#include "WebKitBackendFactory.h"

SDK::Plugin::IPlugin *WebKitBackendFactory::createPlugin(const QString &instancePath) {
    return new WebKitBackend(this, instancePath);
}