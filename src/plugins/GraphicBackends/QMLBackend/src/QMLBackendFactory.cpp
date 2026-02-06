/* @file Фабрика плагина QML Backend. */

// Plugin SDK

#include "QMLBackendFactory.h"

#include <QtCore/QDebug>

#include <SDK/Plugins/PluginFactory.h>

QMLBackendFactory::QMLBackendFactory() {
    mName = "QML graphics backend";
    mDescription = "QML based graphics backend for qml widgets";
    mAuthor = "Humo";
    mVersion = "1.0";
    mModuleName = "qml_backend";
}
