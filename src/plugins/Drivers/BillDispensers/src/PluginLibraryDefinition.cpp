/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include "PluginLibraryDefinition.h"

#include <SDK/Plugins/PluginFactory.h>

BillDispenserPluginFactory::BillDispenserPluginFactory() {
    m_Name = "BillDispenser";
    m_Description = "BillDispenser driver library";
    m_Author = "Humo";
    m_Version = "1.0";
    m_ModuleName = "bill_dispensers"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
