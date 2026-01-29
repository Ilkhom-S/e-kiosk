/* @file Mainline. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWidgets/QApplication>
#include <Common/QtHeadersEnd.h>

// Project
#include "UpdaterSplashScreen.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    UpdaterSplashScreen w;
    w.showFullScreen();
    return a.exec();
}
