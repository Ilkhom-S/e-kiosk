/* @file Реализация фабрики плагинов TemplatePlugin. */

#include "TemplatePluginFactory.h"

#include <QtCore/QDebug>

#include <SDK/Plugins/PluginFactory.h>

#include "TemplatePlugin.h"

TemplatePluginFactory::TemplatePluginFactory() {
    mName = "Template Plugin";
    mDescription = "Minimal template plugin for plugin development";
    mAuthor = "EKiosk Template";
    mVersion = "1.0";
    mModuleName = "template_plugin";

    qDebug() << "TemplatePluginFactory created";
}

//---------------------------------------------------------------------------
// Деструктор фабрики.
/// Выполняет очистку ресурсов фабрики.
TemplatePluginFactory::~TemplatePluginFactory() {
    qDebug() << "TemplatePluginFactory destroyed";
}