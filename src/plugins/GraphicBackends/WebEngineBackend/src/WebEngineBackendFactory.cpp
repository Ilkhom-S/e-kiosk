/* @file Реализация фабрики плагина WebEngineBackend. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPlugin.h>

// Project
#include "WebEngineBackend.h"
#include "WebEngineBackendFactory.h"

WebEngineBackendFactory::WebEngineBackendFactory()
{
    mModuleName = "webengine_backend";
    mName = "WebEngineBackend";
    mDescription = "WebEngine based graphics backend for HTML widgets";
    mAuthor = "Humo";
    mVersion = "1.0";
}