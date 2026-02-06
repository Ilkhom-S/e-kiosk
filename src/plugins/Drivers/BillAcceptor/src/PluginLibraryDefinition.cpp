/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include "PluginLibraryDefinition.h"

#include <SDK/Plugins/PluginFactory.h>

BillAcceptorPluginFactory::BillAcceptorPluginFactory() {
    mName = "BillAcceptor";
    mDescription = "BillAcceptor driver library, CCNet protocol";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "bill_acceptors"; // Название dll/so модуля без расширения
}

//------------------------------------------------------------------------------
