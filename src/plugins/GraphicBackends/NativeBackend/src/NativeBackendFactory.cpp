/* @file Реализация фабрики плагина нативного бэкенда. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

// Project
#include "NativeBackend.h"
#include "NativeBackendFactory.h"

QString SDK::Plugin::PluginFactory::mName = "Native QWidget graphics backend";
QString SDK::Plugin::PluginFactory::mDescription = "Graphics backend for widgets implemented as binary plugins";
QString SDK::Plugin::PluginFactory::mAuthor = "Humo";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName = "native_backend";

NativeBackendFactory::NativeBackendFactory() {
}

//---------------------------------------------------------------------------
// Деструктор фабрики
NativeBackendFactory::~NativeBackendFactory() {
}

//---------------------------------------------------------------------------
// Возвращает название плагина
QString NativeBackendFactory::getName() const {
    return "Native QWidget graphics backend";
}

//---------------------------------------------------------------------------
// Возвращает описание плагина
QString NativeBackendFactory::getDescription() const {
    return "Graphics backend for widgets implemented as binary plugins";
}

//---------------------------------------------------------------------------
// Возвращает автора плагина
QString NativeBackendFactory::getAuthor() const {
    return "Humo";
}

//---------------------------------------------------------------------------
// Возвращает версию плагина
QString NativeBackendFactory::getVersion() const {
    return "1.0";
}

//---------------------------------------------------------------------------
// Возвращает список поддерживаемых плагинов
QStringList NativeBackendFactory::getPluginList() const {
    return QStringList() << "NativeBackend.Instance";
}

//---------------------------------------------------------------------------
// Создает экземпляр плагина
SDK::Plugin::IPlugin *NativeBackendFactory::createPlugin(const QString &instancePath, const QString &configPath) {
    return new NativeBackend(this, instancePath);
}

//---------------------------------------------------------------------------