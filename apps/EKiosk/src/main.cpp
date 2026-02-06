/* @file Mainline. */

#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>
#endif

#include <string.h>

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

        // Чтобы заставить работать QPixmap в потоке, отличном от потока gui (см.
        // qt_pixmap_thread_test() в qpixmap.cpp)
        // TODO PORT_QT5
        // QApplication::setGraphicsSystem("raster");

        qputenv("QMLSCENE_DEVICE", "softwarecontext");

        PPApplication application(Humo::Application, Humo::getVersion(), argc, argv);

        // TODO: restore breakpad integration when compatible with SingleApplication
        // QBreakpadInstance.setDumpPath(PPApplication::getInstance()->getWorkingDirectory() +
        // "/logs/");

        // CatchUnhandledExceptions();

        if (application.getQtApplication().isSecondary()) {
            result = application.exec();
        } else {
            qDebug() << "Already running.";
        }
    } catch (std::exception &aException) {
        // TODO: нет лога, так как программа убит
        // EXCEPTION_FILTER_NO_THROW(???);
        std::cout << "Exited due to exception: " << aException.what();

        if (!result) {
            result = ExitCode::Error;
        }
    }

    qInstallMessageHandler(0);

    ILog::getInstance(Humo::Application)
        ->write(LogLevel::Debug, QString("Exit main() with %1 result.").arg(result));

    return result;
}

//---------------------------------------------------------------------------
