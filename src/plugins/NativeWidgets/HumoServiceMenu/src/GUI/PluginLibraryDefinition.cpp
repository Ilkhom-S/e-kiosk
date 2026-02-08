/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include "PluginLibraryDefinition.h"

#include <SDK/Plugins/PluginFactory.h>

ServiceMenuPluginFactory::ServiceMenuPluginFactory() {
    m_Name = "Service menu native widget";
    m_Description = "Service menu.";
    m_Author = "Humo";
    m_Version = "1.0";
    m_ModuleName = "service_menu"; // Название dll/so модуля без расширения
}

//--------------------------------------------------------------------------
