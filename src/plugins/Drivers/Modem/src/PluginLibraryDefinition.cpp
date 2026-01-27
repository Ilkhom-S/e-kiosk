/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "PluginLibraryDefinition.h"

ModemsPluginFactory::ModemsPluginFactory() {
    mName = "Modems";
    mDescription = "Modem driver library.";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "modems"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
