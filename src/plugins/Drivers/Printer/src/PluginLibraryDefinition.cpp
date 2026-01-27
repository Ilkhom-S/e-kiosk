/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "PluginLibraryDefinition.h"

PrintersPluginFactory::PrintersPluginFactory()
{
    mName = "Printers";
    mDescription = "Printer driver library.";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "printers"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
