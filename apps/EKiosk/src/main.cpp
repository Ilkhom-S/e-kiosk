// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QProcess>
#include <QtWidgets/QApplication>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/BasicApplication.h>

// Project
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication qtApp(argc, argv);

    BasicApplication app(QString(), QString(), argc, argv);

    // if (!app.isPrimaryInstance())
    // {
    //     return 1;
    // }

    auto arguments = qtApp.arguments();
    auto fileName = arguments.at(0);

    QFileInfo fi(fileName);

    if (fi.fileName().toLower() != "ekiosk.exe")
    {
        return 0;
    }

    MainWindow w;

    // Если sheller не запущен, то запускаем
    // DEPRECATED: we migrate to guard instead of this
    if (!w.hasProcess(sheller))
    {
        QProcess::startDetached(sheller, QStringList());
    }

    w.testMode = app.isTestMode();

    if (!w.testMode)
    {
        qtApp.setOverrideCursor(Qt::BlankCursor);
    }

    w.init();

    return qtApp.exec();
}
