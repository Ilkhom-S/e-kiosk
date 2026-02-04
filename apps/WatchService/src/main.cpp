/* @file Реализация сторожевого сервиса как обычного приложения. */

// STL
#include <cstdlib>
#include <iostream>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWidgets/QApplication>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/BasicApplication.h>
#include <Common/Version.h>

// Project
#include "WatchService.h"
#include <singleapplication.h>

namespace CWatchService
{
    const QString Name = "WatchService";
} // namespace CWatchService

//----------------------------------------------------------------------------
void qtMessageHandler(QtMsgType /*aType*/, const QMessageLogContext & /*aContext*/, const QString &aMessage)
{
    static ILog *log = ILog::getInstance("QtMessages");

    log->write(LogLevel::Normal, aMessage);
}

//----------------------------------------------------------------------------
// Точка входа в приложение сторожевого сервиса
int main(int aArgc, char *aArgv[])
{
    BasicQtApplication<SingleApplication> application(CWatchService::Name, Humo::getVersion(), aArgc, aArgv);

    // TODO: restore breakpad integration when compatible with SingleApplication
    // QBreakpadInstance.setDumpPath(application.getWorkingDirectory() + "/logs/");

    // Перенаправляем логи
    ILog *mainLog = application.getLog();
    ILog::getInstance("ConfigManager")->setDestination(mainLog->getDestination());
    ILog::getInstance("CryptEngine")->setDestination(mainLog->getDestination());
    ILog::getInstance("QtMessages")->setDestination(mainLog->getDestination());

    qInstallMessageHandler(qtMessageHandler);

    WatchService service;

    int result = application.exec();

    qInstallMessageHandler(nullptr);

    return result;
}

//----------------------------------------------------------------------------
