/* @file Конфигурация фабрики плагинов. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

QString SDK::Plugin::PluginFactory::mName = "CardReader";
QString SDK::Plugin::PluginFactory::mDescription = "Driver for card reader";
QString SDK::Plugin::PluginFactory::mAuthor = "Humo";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName = "card_readers"; // Название dll/so модуля без расширения

//------------------------------------------------------------------------------
