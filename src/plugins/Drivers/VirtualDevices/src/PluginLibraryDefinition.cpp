/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

VirtualBillAcceptorPluginFactory::VirtualBillAcceptorPluginFactory()
{
    mName = "VirtualDevices";
    mDescription = "Driver for virtual devices.";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "virtual_devices"; // Название dll/so модуля без расширения
}