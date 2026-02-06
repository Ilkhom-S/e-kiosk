/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include "PluginLibraryDefinition.h"

#include <SDK/Plugins/PluginFactory.h>

ServiceMenuPluginFactory::ServiceMenuPluginFactory() {
    mName = "Service menu native widget";
    mDescription = "Service menu.";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "service_menu"; // Название dll/so модуля без расширения
}

//--------------------------------------------------------------------------
