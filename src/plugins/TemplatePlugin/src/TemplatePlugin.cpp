/* @file Реализация плагина TemplatePlugin. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPluginEnvironment.h>

// Project
#include "TemplatePlugin.h"

TemplatePlugin::TemplatePlugin(SDK::Plugin::IEnvironment *aEnvironment, const QString &aInstancePath)
    : mEnvironment(aEnvironment), mInstancePath(aInstancePath), mHelloMessage("Hello from Template Plugin!") {
    qDebug() << "TemplatePlugin created with instance path:" << aInstancePath;
}

//---------------------------------------------------------------------------
// Деструктор плагина.
/// Выполняет очистку ресурсов.
TemplatePlugin::~TemplatePlugin() {
    qDebug() << "TemplatePlugin destroyed";
}

//---------------------------------------------------------------------------
// Возвращает название плагина.
/// @return QString с названием плагина
QString TemplatePlugin::getPluginName() const {
    return "Template Plugin";
}

//---------------------------------------------------------------------------
// Возвращает имя файла конфигурации.
/// @return QString с именем конфигурации
QString TemplatePlugin::getConfigurationName() const {
    return mInstancePath;
}

//---------------------------------------------------------------------------
// Возвращает текущую конфигурацию.
/// @return QVariantMap с параметрами плагина
QVariantMap TemplatePlugin::getConfiguration() const {
    return mConfiguration;
}

//---------------------------------------------------------------------------
// Устанавливает новую конфигурацию.
/// @param aConfiguration Новые параметры конфигурации
void TemplatePlugin::setConfiguration(const QVariantMap &aConfiguration) {
    mConfiguration = aConfiguration;
    qDebug() << "TemplatePlugin configuration set:" << aConfiguration;
}

//---------------------------------------------------------------------------
// Сохраняет конфигурацию.
/// @return true если сохранение успешно
bool TemplatePlugin::saveConfiguration() {
    // В реальном плагине здесь сохраняем в постоянное хранилище
    // For template, just return true
    qDebug() << "TemplatePlugin saveConfiguration called";
    return true;
}

//---------------------------------------------------------------------------
// Проверяет готовность плагина.
/// @return true если плагин готов к работе
bool TemplatePlugin::isReady() const {
    return true;
}

//---------------------------------------------------------------------------
// Возвращает приветственное сообщение.
/// @return QString с сообщением
QString TemplatePlugin::getHelloMessage() const {
    return mHelloMessage;
}