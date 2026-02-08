/* @file Конфигурация фабрики. */

// Plugin SDK

#include <SDK/Plugins/PluginFactory.h>

#include "PluginLibraryDefinition.h"

UcsPluginFactory::UcsPluginFactory() {
    m_Name = "ucs";
    m_Description = "Scenario backend for UCS Pay System";
    m_Author = "Humo";
    m_Version = "1.0";
    m_ModuleName = "ucs";
}

//------------------------------------------------------------------------------
