/* @file Реализация фабрики плагина рекламы. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Project
#include "AdPluginFactory.h"
#include "AdPluginImpl.h"

AdPluginFactory::AdPluginFactory()
{
    mModuleName = "ad_plugin";
    mName = "Ad Plugin";
    mDescription = "Advertisement management plugin for EKiosk";
    mAuthor = "HUMO";
    mVersion = "1.0";
}
