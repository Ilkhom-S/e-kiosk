/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include "PluginLibraryDefinition.h"

#include <SDK/Plugins/PluginFactory.h>

VirtualBillAcceptorPluginFactory::VirtualBillAcceptorPluginFactory() {
    m_Name = "VirtualDevices";
    m_Description = "Driver for virtual devices.";
    m_Author = "Humo";
    m_Version = "1.0";
    m_ModuleName = "virtual_devices"; // Название dll/so модуля без расширения
}