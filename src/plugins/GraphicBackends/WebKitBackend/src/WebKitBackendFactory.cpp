/* @file Реализация фабрики плагина WebKitBackend. */

#include "WebKitBackendFactory.h"

#include <QtCore/QCoreApplication>

#include <SDK/Plugins/IPlugin.h>

#include "WebKitBackend.h"

QString SDK::Plugin::PluginFactory::m_Name = "Webkit graphics backend";
QString SDK::Plugin::PluginFactory::m_Description =
    "Webkit based graphics backend for html widgets (with webkit plugins support)";
QString SDK::Plugin::PluginFactory::m_Author = "Humo";
QString SDK::Plugin::PluginFactory::m_Version = "1.0";
QString SDK::Plugin::PluginFactory::m_ModuleName = "webkit_backend";