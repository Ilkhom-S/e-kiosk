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

//---------------------------------------------------------------------------
// Возвращает название плагина.
/// @return QString с названием плагина
QString TemplatePluginFactory::getName() const {
    return "Template Plugin (Overridden)";
}

//---------------------------------------------------------------------------
// Возвращает описание плагина.
/// @return QString с описанием плагина
QString TemplatePluginFactory::getDescription() const {
    return "Minimal template plugin for plugin development (Overridden)";
}

//---------------------------------------------------------------------------
// Возвращает список доступных плагинов.
/// @return QStringList с именами плагинов
QStringList TemplatePluginFactory::getPluginList() const {
    return QStringList() << "TemplatePlugin.Instance";
}

//---------------------------------------------------------------------------
// Создаёт экземпляр плагина.
/// @param aInstancePath Путь к экземпляру плагина
/// @param aConfigPath Путь к конфигурации
/// @return указатель на созданный плагин или nullptr
SDK::Plugin::IPlugin *TemplatePluginFactory::createPlugin(const QString &aInstancePath, const QString &aConfigPath) {
    qDebug() << "Creating TemplatePlugin with instance path:" << aInstancePath;
    TemplatePlugin *plugin = new TemplatePlugin(this, aInstancePath);
    mCreatedPlugins[plugin] = aInstancePath; // Track the plugin for proper destruction
    return plugin;
}

//---------------------------------------------------------------------------
// Уничтожает экземпляр плагина.
/// @param aPlugin Указатель на плагин для уничтожения
/// @return true если уничтожение успешно
bool TemplatePluginFactory::destroyPlugin(SDK::Plugin::IPlugin *aPlugin) {
    if (aPlugin && mCreatedPlugins.contains(aPlugin)) {
        qDebug() << "Destroying TemplatePlugin:" << aPlugin->getPluginName();
        mCreatedPlugins.remove(aPlugin);
        delete static_cast<TemplatePlugin *>(aPlugin);
        return true;
    }
    return false;
}