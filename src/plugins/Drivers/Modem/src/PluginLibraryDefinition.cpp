/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include "PluginLibraryDefinition.h"

#include <SDK/Plugins/PluginFactory.h>

ModemsPluginFactory::ModemsPluginFactory() {
    m_Name = "Modems";
    m_Description = "Modem driver library.";
    m_Author = "Humo";
    m_Version = "1.0";
    m_ModuleName = "modems"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
