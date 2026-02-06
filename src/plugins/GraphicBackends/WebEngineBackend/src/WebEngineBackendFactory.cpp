/* @file Реализация фабрики плагина WebEngineBackend. */

#include "WebEngineBackendFactory.h"

#include <QtCore/QCoreApplication>

#include <SDK/Plugins/IPlugin.h>

#include "WebEngineBackend.h"

WebEngineBackendFactory::WebEngineBackendFactory() {
    mModuleName = "webengine_backend";
    mName = "WebEngineBackend";
    mDescription = "WebEngine based graphics backend for HTML widgets";
    mAuthor = "Humo";
    mVersion = "1.0";
}