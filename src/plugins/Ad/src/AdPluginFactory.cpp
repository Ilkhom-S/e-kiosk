/* @file Реализация фабрики плагина рекламы. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Project
#include "AdPluginFactory.h"
#include "AdPluginImpl.h"

QString SDK::Plugin::PluginFactory::mName = "Ad Plugin";
QString SDK::Plugin::PluginFactory::mDescription = "Advertisement management plugin for EKiosk";
QString SDK::Plugin::PluginFactory::mAuthor = "HUMO";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName = "ad_plugin";

AdPluginFactory::AdPluginFactory() {
}

//---------------------------------------------------------------------------
// Деструктор фабрики
AdPluginFactory::~AdPluginFactory() {
}

//---------------------------------------------------------------------------
// Возвращает название плагина
QString AdPluginFactory::getName() const {
    return "Ad Plugin";
}

//---------------------------------------------------------------------------
// Возвращает описание плагина
QString AdPluginFactory::getDescription() const {
    return "Advertising content management and scheduling plugin";
}

//---------------------------------------------------------------------------
// Возвращает автора плагина
QString AdPluginFactory::getAuthor() const {
    return "Humo";
}

//---------------------------------------------------------------------------
// Возвращает версию плагина
QString AdPluginFactory::getVersion() const {
    return "1.0";
}

//---------------------------------------------------------------------------
// Возвращает список поддерживаемых плагинов
QStringList AdPluginFactory::getPluginList() const {
    return QStringList() << "AdPlugin.Instance";
}

//---------------------------------------------------------------------------
// Override shutdown to avoid logging during Qt test cleanup
void AdPluginFactory::shutdown() {
    // Call base shutdown but without logging to avoid Qt test cleanup crash
    // The base shutdown logs, which can crash during Qt test framework destruction
    // So we skip the logging part
    mCreatedPlugins.clear();
    mInitialized = false;
}

//---------------------------------------------------------------------------
// Создает экземпляр плагина
SDK::Plugin::IPlugin *AdPluginFactory::createPlugin(const QString &instancePath, const QString &configPath) {
    // Планируем перезагрузку ЕК по истечении рекламы
    return new AdPluginImpl(this, instancePath);
}

//---------------------------------------------------------------------------