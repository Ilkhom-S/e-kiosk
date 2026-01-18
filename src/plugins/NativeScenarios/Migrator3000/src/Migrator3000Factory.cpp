/* @file Фабрика Migrator3000 сценария. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "Migrator3000Factory.h"

QString SDK::Plugin::PluginFactory::mName = "Migrator 3000";
QString SDK::Plugin::PluginFactory::mDescription =
    "Native scenario for automatic migration from 2.x.x to 3.x.x version";
QString SDK::Plugin::PluginFactory::mAuthor = "Humo";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName = "migrator3000";
