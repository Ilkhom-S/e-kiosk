/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "PluginLibraryDefinition.h"

BillDispenserPluginFactory::BillDispenserPluginFactory()
{
    mName = "BillDispenser";
    mDescription = "BillDispenser driver library";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "bill_dispensers"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
