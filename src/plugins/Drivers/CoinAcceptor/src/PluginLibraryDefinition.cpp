/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "PluginLibraryDefinition.h"

CoinAcceptorPluginFactory::CoinAcceptorPluginFactory()
{
    mName = "CoinAcceptor";
    mDescription = "CoinAcceptor driver library";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "coin_acceptors"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
