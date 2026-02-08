/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include "PluginLibraryDefinition.h"

#include <SDK/Plugins/PluginFactory.h>

CoinAcceptorPluginFactory::CoinAcceptorPluginFactory() {
    m_Name = "CoinAcceptor";
    m_Description = "CoinAcceptor driver library";
    m_Author = "Humo";
    m_Version = "1.0";
    m_ModuleName = "coin_acceptors"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
