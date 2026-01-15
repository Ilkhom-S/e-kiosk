/* @file Конфигурация фабрики. */

// Plugin SDK

// SDK
#include <SDK/Plugins/PluginFactory.h>

QString SDK::Plugin::PluginFactory::mName = "Humo payment factory";
QString SDK::Plugin::PluginFactory::mDescription = "";
QString SDK::Plugin::PluginFactory::mAuthor = "Humo";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName = "humo_payment";

Q_EXPORT_PLUGIN2(humo_payment, SDK::Plugin::PluginFactory)

//------------------------------------------------------------------------------
