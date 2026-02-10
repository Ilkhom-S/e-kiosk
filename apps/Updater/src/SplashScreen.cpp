/* @file Вспомогательный экран, закрывающий рабочий стол. */

#include "SplashScreen.h"

#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtGui/QMovie>
#include <QtWidgets/QHBoxLayout>

#include <Common/BasicApplication.h>

#include <algorithm>
#include <boost/bind/bind.hpp>
#include <boost/bind/placeholders.hpp>

SplashScreen::SplashScreen(QWidget *aParent) : QWidget(aParent, Qt::SplashScreen) {
    ui.setupUi(this);
    showMinimized();

    auto *animation = new QMovie(":/images/wait.gif");
    ui.lbAnimation->setMovie(animation);
    animation->start();
}

//----------------------------------------------------------------------------
SplashScreen::~SplashScreen() = default;

//----------------------------------------------------------------------------
void SplashScreen::closeEvent(QCloseEvent *aEvent) {
    aEvent->ignore();
    showMinimized();
}

//----------------------------------------------------------------------------
