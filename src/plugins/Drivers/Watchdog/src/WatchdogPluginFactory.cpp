/* @file Реализация фабрики плагина сторожевых таймеров. */

#include "WatchdogPluginFactory.h"

WatchdogPluginFactory::WatchdogPluginFactory() {
    m_ModuleName = "watchdogs";
    m_Name = "Watchdogs";
    m_Description = "Watchdog driver library.";
    m_Author = "Humo";
    m_Version = "1.0";
}
