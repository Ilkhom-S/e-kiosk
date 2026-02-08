/* @file Шаблон конфигурации фабрики плагинов. */

// Plugin SDK

#include <SDK/Plugins/PluginFactory.h>

#error Измени описание плагина!
QString SDK::Plugin::PluginFactory::m_Name = "My plugin";
QString SDK::Plugin::PluginFactory::m_Description = "Rock'n'Rolling plugin";
QString SDK::Plugin::PluginFactory::m_Author = "Evilcom Software";
QString SDK::Plugin::PluginFactory::m_Version = "1.0";
QString SDK::Plugin::PluginFactory::m_ModuleName =
    "MyPlugin"; // Название dll/so модуля без расширения

//------------------------------------------------------------------------------
