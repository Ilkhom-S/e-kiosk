/* @file Окно настроек. */

#include "TokenServiceWindow.h"

#include <QtCore/QTime>

#include <Common/BasicApplication.h>

#include <SDK/PaymentProcessor/Core/ICore.h>

#include "Backend/HumoServiceBackend.h"
#include "Backend/KeysManager.h"
#include "MessageBox/MessageBox.h"

TokenServiceWindow::TokenServiceWindow(HumoServiceBackend *aBackend, QWidget *aParent)
    : QFrame(aParent), ServiceWindowBase(aBackend) {
    setupUi(this);

    mWindow = new TokenWindow(aBackend, this);

    connect(mWindow, SIGNAL(beginFormat()), SLOT(onBeginFormat()));
    connect(mWindow, SIGNAL(endFormat()), SLOT(onEndFormat()));
    connect(mWindow, SIGNAL(error(QString)), SLOT(onError(QString)));

    mWindow->setParent(this);
    wContainer->setLayout(new QHBoxLayout);
    wContainer->layout()->setSpacing(0);
    wContainer->layout()->setContentsMargins(0, 0, 0, 0);
    wContainer->layout()->addWidget(mWindow);
}

//------------------------------------------------------------------------
bool TokenServiceWindow::activate() {
    mWindow->initialize(mBackend->getKeysManager()->tokenStatus());

    mUIUpdateTimer = startTimer(1000);

    return true;
}

//------------------------------------------------------------------------
bool TokenServiceWindow::deactivate() {
    killTimer(mUIUpdateTimer);

    return true;
}

//------------------------------------------------------------------------
bool TokenServiceWindow::initialize() {
    mWindow->initialize(mBackend->getKeysManager()->tokenStatus());

    return true;
}

//------------------------------------------------------------------------
bool TokenServiceWindow::shutdown() {
    killTimer(mUIUpdateTimer);
    return true;
}

//------------------------------------------------------------------------
void TokenServiceWindow::onBeginFormat() {
    if (GUI::MessageBox::question(tr("#question_format_token_warning"))) {
        GUI::MessageBox::hide();
        GUI::MessageBox::wait(tr("#format_token"));

        killTimer(mUIUpdateTimer);

        mWindow->doFormat();
    }
}

//------------------------------------------------------------------------
void TokenServiceWindow::onEndFormat() {
    GUI::MessageBox::hide();

    mUIUpdateTimer = startTimer(1000);
}

//------------------------------------------------------------------------
void TokenServiceWindow::onError(QString aError) {
    GUI::MessageBox::hide();
    GUI::MessageBox::critical(aError);

    mUIUpdateTimer = startTimer(1000);
}

//------------------------------------------------------------------------
void TokenServiceWindow::timerEvent(QTimerEvent *) {
    mWindow->initialize(mBackend->getKeysManager()->tokenStatus());
}

//------------------------------------------------------------------------
