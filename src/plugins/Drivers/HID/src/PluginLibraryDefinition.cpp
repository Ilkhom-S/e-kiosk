/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

#include <SDK/Plugins/PluginFactory.h>

QString SDK::Plugin::PluginFactory::m_Name = "Scanners";
QString SDK::Plugin::PluginFactory::m_Description = "Scanner driver library.";
QString SDK::Plugin::PluginFactory::m_Author = "Humo";
QString SDK::Plugin::PluginFactory::m_Version = "1.0";
QString SDK::Plugin::PluginFactory::m_ModuleName =
    "scanners"; // Название dll/so модуля без расширения

//------------------------------------------------------------------------------