/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include "PluginLibraryDefinition.h"

#include <SDK/Plugins/PluginFactory.h>

PrintersPluginFactory::PrintersPluginFactory() {
    m_Name = "Printers";
    m_Description = "Printer driver library.";
    m_Author = "Humo";
    m_Version = "1.0";
    m_ModuleName = "printers"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
