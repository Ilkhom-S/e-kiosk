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

QString SDK::Plugin::PluginFactory::mModuleName = "webengine_backend";
QString SDK::Plugin::PluginFactory::mName = "WebEngineBackend";
QString SDK::Plugin::PluginFactory::mDescription = "WebEngine based graphics backend for HTML widgets";
QString SDK::Plugin::PluginFactory::mAuthor = "Humo";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";