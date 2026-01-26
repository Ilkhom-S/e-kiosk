/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "PluginLibraryDefinition.h"

BillAcceptorPluginFactory::BillAcceptorPluginFactory() {
    mName = "BillAcceptor";
    mDescription = "BillAcceptor driver library, CCNet protocol";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "bill_acceptors"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
