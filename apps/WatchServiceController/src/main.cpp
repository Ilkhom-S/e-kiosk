/* @file Реализация модуля управления сторожевым сервисом через сокет. */

// STL
#include <cstdlib>
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

// System
#include <DebugUtils/DebugUtils.h>

// Project
#include "WatchServiceController.h"
#include <singleapplication.h>

int main(int aArgc, char *aArgv[])
{
    BasicQtApplication<SingleApplication> application("WatchServiceController", Humo::getVersion(), aArgc, aArgv);

    // Логер: явный лог старта
    LOG(application.getLog(), LogLevel::Normal, "[tray] main() started");

    // Load translations
    QTranslator translator;
    QString locale = QLocale::system().name();
    QString qmFile = QString("WatchServiceController_%1.qm").arg(locale);
    QString qmPath = QDir(QApplication::applicationDirPath()).absoluteFilePath("locale/" + qmFile);
    if (translator.load(qmPath))
    {
        QApplication::installTranslator(&translator);
    }

    // Перенаправляем логи.
    ILog::getInstance(CIMessageQueueClient::DefaultLog)->setDestination(application.getLog()->getName());

    // application.getQtApplication().initialize();
    application.getQtApplication().setQuitOnLastWindowClosed(false);
#ifndef Q_OS_MACOS
    application.getQtApplication().setWindowIcon(QIcon(":/icons/tray-monogram.png"));
#endif

#ifdef Q_OS_WIN
    // On Windows, ensure no windows are shown in taskbar for pure tray apps
    application.getQtApplication().setAttribute(Qt::AA_DontShowIconsInTaskbar);
#endif

    WatchServiceController controller;

    LOG(application.getLog(), LogLevel::Normal, "[tray] entering event loop");
    return application.exec();
}
