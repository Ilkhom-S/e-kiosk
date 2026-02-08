/* @file Окно настройки ключей. */

#include "KeysWizardPage.h"

#include "Backend/KeysManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "GUI/KeysWindow.h"
#include "GUI/MessageBox/MessageBox.h"

KeysWizardPage::KeysWizardPage(ServiceMenuBackend *aBackend, QWidget *aParent)
    : WizardPageBase(aBackend, aParent) {
    m_KeysWindow = new KeysWindow(aBackend, this);

    connect(m_KeysWindow, SIGNAL(beginGenerating()), SLOT(onBeginGenerating()));
    connect(m_KeysWindow, SIGNAL(endGenerating()), SLOT(onEndGenerating()));
    connect(m_KeysWindow, SIGNAL(error(QString)), SLOT(onError(QString)));

    setLayout(new QHBoxLayout(this));
    layout()->setSpacing(0);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(m_KeysWindow);
}

//------------------------------------------------------------------------
bool KeysWizardPage::initialize() {
    emit pageEvent("#can_proceed", false);

    auto tokenStatus = m_Backend->getKeysManager()->tokenStatus();

    m_KeysWindow->initialize(tokenStatus.available, tokenStatus.initialized);

    return true;
}

//------------------------------------------------------------------------
bool KeysWizardPage::shutdown() {
    return true;
}

//------------------------------------------------------------------------
bool KeysWizardPage::activate() {
    auto tokenStatus = m_Backend->getKeysManager()->tokenStatus();

    m_KeysWindow->initialize(tokenStatus.available, tokenStatus.initialized);

    return true;
}

//------------------------------------------------------------------------
bool KeysWizardPage::deactivate() {
    return true;
}

//----------------------------------------------------------------------------
void KeysWizardPage::onBeginGenerating() {
    GUI::MessageBox::wait(tr("#creating_keys"));

    m_KeysWindow->doGenerate();
}

//----------------------------------------------------------------------------
void KeysWizardPage::onEndGenerating() {
    GUI::MessageBox::hide();

    // Сохраняем ключи
    if (!m_KeysWindow->save()) {
        GUI::MessageBox::critical(tr("#cannot_save_keys"));
        emit pageEvent("#can_proceed", false);
    } else {
        emit pageEvent("#can_proceed", true);
    }
}

//----------------------------------------------------------------------------
void KeysWizardPage::onError(QString aError) {
    GUI::MessageBox::hide();
    GUI::MessageBox::critical(aError);

    emit pageEvent("#can_proceed", false);
}

//------------------------------------------------------------------------
