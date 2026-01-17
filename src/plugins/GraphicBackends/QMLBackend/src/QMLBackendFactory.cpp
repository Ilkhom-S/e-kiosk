/* @file Фабрика плагина QML Backend. */

// Plugin SDK

// Qt
#include <Common/QtHeadersBegin.h>
#include "QMLBackendFactory.h"
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/PluginFactory.h>

QString SDK::Plugin::PluginFactory::mName = "QML graphics backend";
QString SDK::Plugin::PluginFactory::mDescription = "QML based graphics backend for qml widgets";
QString SDK::Plugin::PluginFactory::mAuthor = "Humo";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName = "qml_backend";

QMLBackendFactory::QMLBackendFactory() {
    qDebug() << "QMLBackendFactory created";
}

QMLBackendFactory::~QMLBackendFactory() {
    qDebug() << "QMLBackendFactory destroyed";
}

QString QMLBackendFactory::getName() const {
    return SDK::Plugin::PluginFactory::mName;
}

QString QMLBackendFactory::getDescription() const {
    return SDK::Plugin::PluginFactory::mDescription;
}