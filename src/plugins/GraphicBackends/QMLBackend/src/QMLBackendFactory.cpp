/* @file Фабрика плагина QML Backend. */

// Plugin SDK

#include "QMLBackendFactory.h"

#include <QtCore/QDebug>

#include <SDK/Plugins/PluginFactory.h>

QMLBackendFactory::QMLBackendFactory() {
    m_Name = "QML graphics backend";
    m_Description = "QML based graphics backend for qml widgets";
    m_Author = "Humo";
    m_Version = "1.0";
    m_ModuleName = "qml_backend";
}
