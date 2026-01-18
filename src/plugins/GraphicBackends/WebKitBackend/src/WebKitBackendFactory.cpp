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

QString SDK::Plugin::PluginFactory::mName = "Webkit graphics backend";
QString SDK::Plugin::PluginFactory::mDescription =
    "Webkit based graphics backend for html widgets (with webkit plugins support)";
QString SDK::Plugin::PluginFactory::mAuthor = "Humo";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName = "webkit_backend";