/* @file Реализация фабрики плагина WebKitBackend. */

#include "WebKitBackendFactory.h"

#include <QtCore/QCoreApplication>

#include <SDK/Plugins/IPlugin.h>

#include "WebKitBackend.h"

QString SDK::Plugin::PluginFactory::mName = "Webkit graphics backend";
QString SDK::Plugin::PluginFactory::mDescription =
    "Webkit based graphics backend for html widgets (with webkit plugins support)";
QString SDK::Plugin::PluginFactory::mAuthor = "Humo";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName = "webkit_backend";