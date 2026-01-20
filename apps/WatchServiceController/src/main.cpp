/* @file Реализация модуля управления сторожевым сервисом через сокет. */

// stl

// STL
#include <iostream>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTranslator>
#include <QtWidgets/QApplication>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/BasicApplication.h>
#include <Common/ExitCodes.h>
#include <Common/Version.h>

// Project
#include "WatchServiceController.h"
#include <singleapplication.h>

int main(int aArgc, char *aArgv[]) {
    BasicQtApplication<SingleApplication> application("WatchServiceController", Humo::getVersion(), aArgc, aArgv);

    if (!application.isPrimaryInstance()) {
        // notify running instance and exit
        application.getQtApplication().sendMessage("Instance!!!");
        LOG(application.getLog(), LogLevel::Warning, "Another instance is already running.");
        return 0;
    }

    // TODO: restore breakpad integration when compatible with SingleApplication

    // Перенаправляем логи.
    ILog::getInstance(CIMessageQueueClient::DefaultLog)->setDestination(application.getLog()->getName());

    // application.getQtApplication().initialize();
    application.getQtApplication().setQuitOnLastWindowClosed(false);

    WatchServiceController controller;

    return application.exec();
}

//----------------------------------------------------------------------------
