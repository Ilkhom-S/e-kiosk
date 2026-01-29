/* @file Реализация фабрики плагина сторожевых таймеров. */

// Project
#include "WatchdogPluginFactory.h"

WatchdogPluginFactory::WatchdogPluginFactory()
{
    mModuleName = "watchdogs";
    mName = "Watchdogs";
    mDescription = "Watchdog driver library.";
    mAuthor = "Humo";
    mVersion = "1.0";
}
