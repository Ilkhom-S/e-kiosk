/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include "PluginLibraryDefinition.h"

#include <SDK/Plugins/PluginFactory.h>

PrintersPluginFactory::PrintersPluginFactory() {
    mName = "Printers";
    mDescription = "Printer driver library.";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "printers"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
