/* @file Реализация фабрики плагина NativeBackend. */

#include "NativeBackendFactory.h"

#include <QtCore/QCoreApplication>

#include <SDK/Plugins/IPlugin.h>

#include "NativeBackend.h"

NativeBackendFactory::NativeBackendFactory() {
    mModuleName = "native_backend";
    mName = "Native Backend";
    mDescription = "Native graphics backend for EKiosk";
    mAuthor = "CPP Static Author Test";
    mVersion = "1.0";
}
