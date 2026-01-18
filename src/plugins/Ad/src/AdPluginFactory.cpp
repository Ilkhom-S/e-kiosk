/* @file Реализация фабрики плагина рекламы. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Project
#include "AdPluginFactory.h"
#include "AdPluginImpl.h"

QString SDK::Plugin::PluginFactory::mName = "Ad Plugin";
QString SDK::Plugin::PluginFactory::mDescription = "Advertisement management plugin for EKiosk";
QString SDK::Plugin::PluginFactory::mAuthor = "HUMO";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName = "ad_plugin";
