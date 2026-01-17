// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDebug>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/PluginFactory.h>

// Project
#include "TestPluginFactory.h"

QString SDK::Plugin::PluginFactory::mName = "Test Plugin";
QString SDK::Plugin::PluginFactory::mDescription = "Minimal test plugin for verifying plugin system";
QString SDK::Plugin::PluginFactory::mAuthor = "EKiosk Test";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName = "test_plugin";

TestPluginFactory::TestPluginFactory() {
    qDebug() << "TestPluginFactory created";
}

TestPluginFactory::~TestPluginFactory() {
    qDebug() << "TestPluginFactory destroyed";
}

QString TestPluginFactory::getName() const {
    return "Test Plugin (Overridden)";
}

QString TestPluginFactory::getDescription() const {
    return "Minimal test plugin for verifying plugin system (Overridden)";
}