/* @file Окно настройки keys. */

#include "KeysWizardPage.h"

KeysWizardPage::KeysWizardPage(HumoServiceBackend *aBackend, QWidget *aParent)
    : WizardPageBase(aBackend, aParent) {}

//----------------------------------------------------------------------------
bool KeysWizardPage::initialize() {
    return true;
}

//----------------------------------------------------------------------------
bool KeysWizardPage::shutdown() {
    return true;
}

//----------------------------------------------------------------------------
bool KeysWizardPage::activate() {
    return true;
}

//----------------------------------------------------------------------------
bool KeysWizardPage::deactivate() {
    return true;
}

//----------------------------------------------------------------------------