/* @file Шаблон конфигурации фабрики плагинов. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

#error Измени описание плагина!
QString SDK::Plugin::PluginFactory::mName = "My plugin";
QString SDK::Plugin::PluginFactory::mDescription = "Rock'n'Rolling plugin";
QString SDK::Plugin::PluginFactory::mAuthor = "Evilcom Software";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName = "MyPlugin"; // Название dll/so модуля без расширения

//------------------------------------------------------------------------------
