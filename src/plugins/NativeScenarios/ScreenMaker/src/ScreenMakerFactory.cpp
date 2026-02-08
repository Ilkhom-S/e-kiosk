/* @file Фабрика плагина ScreenMaker - инициализация метаданных. */

#include "ScreenMakerFactory.h"

ScreenMakerPluginFactory::ScreenMakerPluginFactory() {
    m_Name = "Screenshot maker";
    m_Description = "Native scenario for create and edit ui screenshot";
    m_Author = "Humo";
    m_Version = "1.0";
    m_ModuleName = "screen_maker";
}
