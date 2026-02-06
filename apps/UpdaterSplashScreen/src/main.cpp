/* @file Mainline. */

#include <QtWidgets/QApplication>

#include "UpdaterSplashScreen.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    UpdaterSplashScreen w;
    w.showFullScreen();
    return a.exec();
}
