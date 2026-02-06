/* @file Реализация фабрики плагина рекламы. */

#include "AdPluginFactory.h"

#include <QtCore/QStringList>

#include "AdPluginImpl.h"

AdPluginFactory::AdPluginFactory() {
    mModuleName = "ad_plugin";
    mName = "Ad Plugin";
    mDescription = "Advertisement management plugin for EKiosk";
    mAuthor = "HUMO";
    mVersion = "1.0";
}
