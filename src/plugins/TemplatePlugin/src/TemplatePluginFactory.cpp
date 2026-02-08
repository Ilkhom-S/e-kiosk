/* @file Реализация фабрики плагинов TemplatePlugin. */

#include "TemplatePluginFactory.h"

#include <QtCore/QDebug>

#include <SDK/Plugins/PluginFactory.h>

#include "TemplatePlugin.h"

TemplatePluginFactory::TemplatePluginFactory() {
    m_Name = "Template Plugin";
    m_Description = "Minimal template plugin for plugin development";
    m_Author = "EKiosk Template";
    m_Version = "1.0";
    m_ModuleName = "template_plugin";

    qDebug() << "TemplatePluginFactory created";
}

//---------------------------------------------------------------------------
// Деструктор фабрики.
/// Выполняет очистку ресурсов фабрики.
TemplatePluginFactory::~TemplatePluginFactory() {
    qDebug() << "TemplatePluginFactory destroyed";
}