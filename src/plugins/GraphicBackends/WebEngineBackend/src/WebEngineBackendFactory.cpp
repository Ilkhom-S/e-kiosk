/* @file Реализация фабрики плагина WebEngineBackend. */

#include "WebEngineBackendFactory.h"

#include <QtCore/QCoreApplication>

#include <SDK/Plugins/IPlugin.h>

#include "WebEngineBackend.h"

WebEngineBackendFactory::WebEngineBackendFactory() {
    m_ModuleName = "webengine_backend";
    m_Name = "WebEngineBackend";
    m_Description = "WebEngine based graphics backend for HTML widgets";
    m_Author = "Humo";
    m_Version = "1.0";
}