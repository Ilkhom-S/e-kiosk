/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "PluginLibraryDefinition.h"

ServiceMenuPluginFactory::ServiceMenuPluginFactory()
{
    mName = "Service menu native widget";
    mDescription = "Service menu.";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "service_menu"; // Название dll/so модуля без расширения
}

//--------------------------------------------------------------------------
