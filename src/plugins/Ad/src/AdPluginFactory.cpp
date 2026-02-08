/* @file Реализация фабрики плагина рекламы. */

#include "AdPluginFactory.h"

#include <QtCore/QStringList>

#include "AdPluginImpl.h"

AdPluginFactory::AdPluginFactory() {
    m_ModuleName = "ad_plugin";
    m_Name = "Ad Plugin";
    m_Description = "Advertisement management plugin for EKiosk";
    m_Author = "HUMO";
    m_Version = "1.0";
}
