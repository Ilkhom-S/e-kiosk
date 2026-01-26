/* @file Реализация фабрики плагина NativeBackend. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QCoreApplication>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPlugin.h>

// Project
#include "NativeBackend.h"
#include "NativeBackendFactory.h"

NativeBackendFactory::NativeBackendFactory() {
    mModuleName = "native_backend";
    mName = "Native Backend";
    mDescription = "Native graphics backend for EKiosk";
    mAuthor = "CPP Static Author Test";
    mVersion = "1.0";
}
