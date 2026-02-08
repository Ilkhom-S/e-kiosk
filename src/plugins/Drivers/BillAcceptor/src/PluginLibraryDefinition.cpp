/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include "PluginLibraryDefinition.h"

#include <SDK/Plugins/PluginFactory.h>

BillAcceptorPluginFactory::BillAcceptorPluginFactory() {
    m_Name = "BillAcceptor";
    m_Description = "BillAcceptor driver library, CCNet protocol";
    m_Author = "Humo";
    m_Version = "1.0";
    m_ModuleName = "bill_acceptors"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
