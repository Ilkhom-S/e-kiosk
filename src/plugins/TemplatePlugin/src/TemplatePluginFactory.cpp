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