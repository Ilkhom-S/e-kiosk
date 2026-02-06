/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include "PluginLibraryDefinition.h"

#include <SDK/Plugins/PluginFactory.h>

ModemsPluginFactory::ModemsPluginFactory() {
    mName = "Modems";
    mDescription = "Modem driver library.";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "modems"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
