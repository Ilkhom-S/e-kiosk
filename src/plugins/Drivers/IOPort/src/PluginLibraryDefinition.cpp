/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include "PluginLibraryDefinition.h"

#include <SDK/Plugins/PluginFactory.h>

IOPortsPluginFactory::IOPortsPluginFactory() {
    mName = "IO ports";
    mDescription = "IO ports driver library (serial, parallel and other).";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "ioports"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
