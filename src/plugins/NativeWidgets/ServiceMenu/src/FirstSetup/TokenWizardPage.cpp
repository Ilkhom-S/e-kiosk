/* @file Окно настройки RuToken. */

#include "TokenWizardPage.h"

#include "Backend/KeysManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "GUI/MessageBox/MessageBox.h"
#include "GUI/TokenWindow.h"

TokenWizardPage::TokenWizardPage(ServiceMenuBackend *aBackend, QWidget *aParent)
    : WizardPageBase(aBackend, aParent), m_UIUpdateTimer(0), m_TokenWindow(new TokenWindow(aBackend, this)) {
    

    connect(m_TokenWindow, SIGNAL(beginFormat()), SLOT(onBeginFormat()));
    connect(m_TokenWindow, SIGNAL(endFormat()), SLOT(onEndFormat()));
    connect(m_TokenWindow, SIGNAL(error(QString)), SLOT(onError(QString)));

    setLayout(new QHBoxLayout(this));
    layout()->setSpacing(0);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(m_TokenWindow);
}

//------------------------------------------------------------------------
bool TokenWizardPage::initialize() {
    auto status = m_Backend->getKeysManager()->tokenStatus();

    m_TokenWindow->initialize(status);
    emit pageEvent("#can_proceed", status.isOK());

    if (!status.isOK()) {
        m_UIUpdateTimer = startTimer(1000);
    }

    return true;
}

//------------------------------------------------------------------------
bool TokenWizardPage::shutdown() {
    killTimer(m_UIUpdateTimer);
    return true;
}

//------------------------------------------------------------------------
bool TokenWizardPage::activate() {
    m_TokenWindow->initialize(m_Backend->getKeysManager()->tokenStatus());

    return true;
}

//------------------------------------------------------------------------
bool TokenWizardPage::deactivate() {
    killTimer(m_UIUpdateTimer);
    return true;
}

//----------------------------------------------------------------------------
void TokenWizardPage::onBeginFormat() {
    GUI::MessageBox::hide();
    GUI::MessageBox::wait(tr("#format_token"));

    killTimer(m_UIUpdateTimer);

    m_TokenWindow->doFormat();
}

//----------------------------------------------------------------------------
void TokenWizardPage::onEndFormat() {
    GUI::MessageBox::hide();

    m_UIUpdateTimer = startTimer(1000);
}

//----------------------------------------------------------------------------
void TokenWizardPage::onError(QString aError) {
    GUI::MessageBox::hide();
    GUI::MessageBox::critical(aError);

    m_UIUpdateTimer = startTimer(1000);
}

//------------------------------------------------------------------------
void TokenWizardPage::timerEvent(QTimerEvent * /*event*/) {
    auto status = m_Backend->getKeysManager()->tokenStatus();

    m_TokenWindow->initialize(status);

    emit pageEvent("#can_proceed", status.isOK());
}

//------------------------------------------------------------------------
