/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include "PluginLibraryDefinition.h"

#include <SDK/Plugins/PluginFactory.h>

IOPortsPluginFactory::IOPortsPluginFactory() {
    m_Name = "IO ports";
    m_Description = "IO ports driver library (serial, parallel and other).";
    m_Author = "Humo";
    m_Version = "1.0";
    m_ModuleName = "ioports"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
