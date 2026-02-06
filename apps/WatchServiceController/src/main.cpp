/* @file Реализация модуля управления сторожевым сервисом через сокет. */

#include <QtCore/QTranslator>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMessageBox>

#include <Common/BasicApplication.h>
#include <Common/ExitCodes.h>
#include <Common/SafeApplication.h>
#include <Common/Version.h>

#include <DebugUtils/DebugUtils.h>
#include <cstdlib>
#include <iostream>

#include "WatchServiceController.h"

int main(int aArgc, char *aArgv[]) {

    BasicQtApplication<SingleApplication> application(
        "WatchServiceController", Humo::getVersion(), aArgc, aArgv);

#ifndef Q_OS_MACOS
    // Check for single instance BEFORE creating QApplication
    if (application.getQtApplication().isSecondary()) {
        // single_instance_guard.sendMessage(application.getQtApplication().arguments().join('
        // ').toUtf8());
        qDebug() << "App already running.";
        qDebug() << "Primary instance PID: " << application.getQtApplication().primaryPid();
        qDebug() << "Primary instance user: " << application.getQtApplication().primaryUser();
        LOG(application.getLog(), LogLevel::Warning, "Another instance is already running.");
        return 0;
    }
#endif

    // Перенаправляем логи.
    ILog::getInstance(CIMessageQueueClient::DefaultLog)
        ->setDestination(application.getLog()->getName());

    application.getQtApplication().setQuitOnLastWindowClosed(false);
#ifndef Q_OS_MACOS
    application.getQtApplication().setWindowIcon(QIcon(":/icons/controller-monogramTemplate.png"));
#endif

#ifdef Q_OS_WIN
    // On Windows, ensure no windows are shown in taskbar for pure tray apps
    application.getQtApplication().setAttribute(Qt::AA_DontShowIconsInTaskbar);
#endif

    WatchServiceController controller;

    return application.exec();
}
