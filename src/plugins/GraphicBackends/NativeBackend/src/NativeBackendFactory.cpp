/* @file Реализация фабрики плагина NativeBackend. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPlugin.h>

// Project
#include "NativeBackend.h"
#include "NativeBackendFactory.h"

QString SDK::Plugin::PluginFactory::mModuleName = "native_backend";
QString SDK::Plugin::PluginFactory::mName = "Native Backend";
QString SDK::Plugin::PluginFactory::mDescription = "Native graphics backend for EKiosk";
QString SDK::Plugin::PluginFactory::mAuthor = "CPP Static Author Test";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
