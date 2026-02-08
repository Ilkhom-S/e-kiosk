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

    m_Window = new TokenWindow(aBackend, this);

    connect(m_Window, SIGNAL(beginFormat()), SLOT(onBeginFormat()));
    connect(m_Window, SIGNAL(endFormat()), SLOT(onEndFormat()));
    connect(m_Window, SIGNAL(error(QString)), SLOT(onError(QString)));

    m_Window->setParent(this);
    wContainer->setLayout(new QHBoxLayout);
    wContainer->layout()->setSpacing(0);
    wContainer->layout()->setContentsMargins(0, 0, 0, 0);
    wContainer->layout()->addWidget(m_Window);
}

//------------------------------------------------------------------------
bool TokenServiceWindow::activate() {
    m_Window->initialize(m_Backend->getKeysManager()->tokenStatus());

    m_UIUpdateTimer = startTimer(1000);

    return true;
}

//------------------------------------------------------------------------
bool TokenServiceWindow::deactivate() {
    killTimer(m_UIUpdateTimer);

    return true;
}

//------------------------------------------------------------------------
bool TokenServiceWindow::initialize() {
    m_Window->initialize(m_Backend->getKeysManager()->tokenStatus());

    return true;
}

//------------------------------------------------------------------------
bool TokenServiceWindow::shutdown() {
    killTimer(m_UIUpdateTimer);
    return true;
}

//------------------------------------------------------------------------
void TokenServiceWindow::onBeginFormat() {
    if (GUI::MessageBox::question(tr("#question_format_token_warning"))) {
        GUI::MessageBox::hide();
        GUI::MessageBox::wait(tr("#format_token"));

        killTimer(m_UIUpdateTimer);

        m_Window->doFormat();
    }
}

//------------------------------------------------------------------------
void TokenServiceWindow::onEndFormat() {
    GUI::MessageBox::hide();

    m_UIUpdateTimer = startTimer(1000);
}

//------------------------------------------------------------------------
void TokenServiceWindow::onError(QString aError) {
    GUI::MessageBox::hide();
    GUI::MessageBox::critical(aError);

    m_UIUpdateTimer = startTimer(1000);
}

//------------------------------------------------------------------------
void TokenServiceWindow::timerEvent(QTimerEvent *) {
    m_Window->initialize(m_Backend->getKeysManager()->tokenStatus());
}

//------------------------------------------------------------------------
