/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

QString SDK::Plugin::PluginFactory::mName = "VirtualDevices";
QString SDK::Plugin::PluginFactory::mDescription = "Driver for virtual devices.";
QString SDK::Plugin::PluginFactory::mAuthor = "Humo";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName = "virtual_devices"; // Название dll/so модуля без расширения

//------------------------------------------------------------------------------