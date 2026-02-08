/* @file Реализация фабрики плагина NativeBackend. */

#include "NativeBackendFactory.h"

#include <QtCore/QCoreApplication>

#include <SDK/Plugins/IPlugin.h>

#include "NativeBackend.h"

NativeBackendFactory::NativeBackendFactory() {
    m_ModuleName = "native_backend";
    m_Name = "Native Backend";
    m_Description = "Native graphics backend for EKiosk";
    m_Author = "CPP Static Author Test";
    m_Version = "1.0";
}
