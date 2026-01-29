/* @file Mainline. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <Common/QtHeadersEnd.h>

// Project
#include "UpdaterApp.h"

int main(int aArgc, char *aArgv[])
{
    qInstallMessageHandler(UpdaterApp::qtMessageHandler);

    UpdaterApp app(aArgc, aArgv);
    // TODO: restore breakpad integration when compatible with SingleApplication
    // QBreakpadInstance.setDumpPath(app.getWorkingDirectory() + "/logs/");

    QCoreApplication::setLibraryPaths(QStringList() << QCoreApplication::applicationDirPath());

    app.run();

    app.getLog()->write(LogLevel::Normal, QString("Exit with resultCode = %1").arg(app.getResultCode()));

    return app.getResultCode();
}

//---------------------------------------------------------------------------
