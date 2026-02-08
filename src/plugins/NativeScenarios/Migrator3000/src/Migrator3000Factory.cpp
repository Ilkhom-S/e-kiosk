/* @file Фабрика Migrator3000 сценария. */

// Plugin SDK

#include "Migrator3000Factory.h"

#include <SDK/Plugins/PluginFactory.h>

Migrator3000Factory::Migrator3000Factory() {
    m_ModuleName = "migrator3000";
    m_Name = "Migrator 3000";
    m_Description = "Native scenario for automatic migration from 2.x.x to 3.x.x version";
    m_Author = "Humo";
    m_Version = "1.0";
}
