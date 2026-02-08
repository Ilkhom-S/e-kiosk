/* @file Конфигурация фабрики. */

// Plugin SDK

#include <SDK/Plugins/PluginFactory.h>

#include "PluginLibraryDefinition.h"

UnitellerPluginFactory::UnitellerPluginFactory() {
    m_Name = "uniteller";
    m_Description = "Native scenario for Uniteller";
    m_Author = "Humo";
    m_Version = "1.0";
    m_ModuleName = "uniteller";
}

//------------------------------------------------------------------------------
