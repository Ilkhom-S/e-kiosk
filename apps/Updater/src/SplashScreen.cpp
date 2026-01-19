/* @file Вспомогательный экран, закрывающий рабочий стол. */

// STL
#include <algorithm>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtGui/QMovie>
#include <QtWidgets/QHBoxLayout>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/Application.h>

// ThirdParty
#include <boost/bind.hpp>
#include <boost/bind/placeholders.hpp>

// Project
#include "SplashScreen.h"

SplashScreen::SplashScreen(QWidget *aParent) : QWidget(aParent, Qt::SplashScreen) {
    ui.setupUi(this);
    showMinimized();

    QMovie *animation = new QMovie(":/images/wait.gif");
    ui.lbAnimation->setMovie(animation);
    animation->start();
}

//----------------------------------------------------------------------------
SplashScreen::~SplashScreen() {
}

//----------------------------------------------------------------------------
void SplashScreen::closeEvent(QCloseEvent *aEvent) {
    aEvent->ignore();
    showMinimized();
}

//----------------------------------------------------------------------------
