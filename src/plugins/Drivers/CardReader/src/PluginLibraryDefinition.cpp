/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include "PluginLibraryDefinition.h"

#include <SDK/Plugins/PluginFactory.h>

CardReaderPluginFactory::CardReaderPluginFactory() {
    m_Name = "CardReader";
    m_Description = "Driver for card reader";
    m_Author = "Humo";
    m_Version = "1.0";
    m_ModuleName = "card_readers"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
