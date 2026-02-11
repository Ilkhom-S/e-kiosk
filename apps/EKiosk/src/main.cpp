/* @file Mainline. */

#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#endif

#include <cstring>

// stl
#include <QtCore/QDateTime>
#include <QtCore/QLoggingCategory>
#include <QtCore/QtGlobal>

#include <iostream>
// #include <QBreakpadHandler.h>

#include <Common/ExitCodes.h>
#include <Common/Version.h>

#include "System/PPApplication.h"
#include "System/UnhandledException.h"

//---------------------------------------------------------------------------
int main(int argc, char **argv) {
    int result = 0;

    try {
        qInstallMessageHandler(PPApplication::qtMessageHandler);

        // Qt5: Use software renderer for Qt Quick to avoid GPU issues
        // Qt6: Uses RHI (Rendering Hardware Interface) with software backend
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        qputenv("QMLSCENE_DEVICE", "softwarecontext");
#else
        qputenv("QSG_RHI_BACKEND", "software");
#endif

        PPApplication application(Humo::Application, Humo::getVersion(), argc, argv);

        // TODO: restore breakpad integration when compatible with SingleApplication
        // QBreakpadInstance.setDumpPath(PPApplication::getInstance()->getWorkingDirectory() +
        // "/logs/");

        // CatchUnhandledExceptions();

#ifndef Q_OS_MACOS
        // SingleApplication has known issues on macOS - skip instance check
        if (application.getQtApplication().isSecondary()) {
            qDebug() << "App already running.";
            qDebug() << "Primary instance PID: " << application.getQtApplication().primaryPid();
            qDebug() << "Primary instance user: " << application.getQtApplication().primaryUser();
            return 0;
        }
#endif

        result = application.exec();
    } catch (std::exception &aException) {
        // TODO: нет лога, так как программа убит
        // EXCEPTION_FILTER_NO_THROW(???);
        std::cout << "Exited due to exception: " << aException.what();

        if (result == 0) {
            result = ExitCode::Error;
        }
    }

    qInstallMessageHandler(nullptr);

    ILog::getInstance(Humo::Application)
        ->write(LogLevel::Debug, QString("Exit main() with %1 result.").arg(result));

    return result;
}

//---------------------------------------------------------------------------
