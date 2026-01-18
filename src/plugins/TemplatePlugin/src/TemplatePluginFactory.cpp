/* @file Реализация фабрики плагинов TemplatePlugin. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "TemplatePlugin.h"
#include "TemplatePluginFactory.h"

QString SDK::Plugin::PluginFactory::mName = "Template Plugin";
QString SDK::Plugin::PluginFactory::mDescription = "Minimal template plugin for plugin development";
QString SDK::Plugin::PluginFactory::mAuthor = "EKiosk Template";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName = "template_plugin";

//---------------------------------------------------------------------------
// Конструктор фабрики.
/// Выполняет инициализацию фабрики плагинов.
TemplatePluginFactory::TemplatePluginFactory() {
    qDebug() << "TemplatePluginFactory created";
}

//---------------------------------------------------------------------------
// Деструктор фабрики.
/// Выполняет очистку ресурсов фабрики.
TemplatePluginFactory::~TemplatePluginFactory() {
    qDebug() << "TemplatePluginFactory destroyed";
}