/* @file Конфигурация фабрики. */

// Plugin SDK

#include <SDK/Plugins/PluginFactory.h>

#include "PluginLibraryDefinition.h"

UcsPluginFactory::UcsPluginFactory() {
    mName = "ucs";
    mDescription = "Scenario backend for UCS Pay System";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "ucs";
}

//------------------------------------------------------------------------------
