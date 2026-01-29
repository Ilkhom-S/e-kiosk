/* @file Конфигурация фабрики. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "PluginLibraryDefinition.h"

UnitellerPluginFactory::UnitellerPluginFactory()
{
    mName = "uniteller";
    mDescription = "Native scenario for Uniteller";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "uniteller";
}

//------------------------------------------------------------------------------
