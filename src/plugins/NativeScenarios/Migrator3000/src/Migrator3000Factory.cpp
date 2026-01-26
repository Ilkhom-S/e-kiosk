/* @file Фабрика Migrator3000 сценария. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "Migrator3000Factory.h"

Migrator3000Factory::Migrator3000Factory() {
    mModuleName = "migrator3000";
    mName = "Migrator 3000";
    mDescription = "Native scenario for automatic migration from 2.x.x to 3.x.x version";
    mAuthor = "Humo";
    mVersion = "1.0";
}
