/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "PluginLibraryDefinition.h"

CardReaderPluginFactory::CardReaderPluginFactory() {
    mName = "CardReader";
    mDescription = "Driver for card reader";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "card_readers"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
