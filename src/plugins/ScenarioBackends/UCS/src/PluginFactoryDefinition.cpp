/* @file Конфигурация фабрики. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "PluginLibraryDefinition.h"

UcsPluginFactory::UcsPluginFactory()
{
    mName = "ucs";
    mDescription = "Scenario backend for UCS Pay System";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "ucs";
}

//------------------------------------------------------------------------------
