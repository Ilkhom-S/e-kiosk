/* @file Фабрика плагина ScreenMaker - инициализация метаданных. */

// Project
#include "ScreenMakerFactory.h"

ScreenMakerPluginFactory::ScreenMakerPluginFactory()
{
    mName = "Screenshot maker";
    mDescription = "Native scenario for create and edit ui screenshot";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "screen_maker";
}
