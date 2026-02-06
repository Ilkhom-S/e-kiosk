/* @file Окно настройки dialup. */

#include "DialupWizardPage.h"

DialupWizardPage::DialupWizardPage(HumoServiceBackend *aBackend, QWidget *aParent)
    : WizardPageBase(aBackend, aParent) {}

//----------------------------------------------------------------------------
bool DialupWizardPage::initialize() {
    return true;
}

//----------------------------------------------------------------------------
bool DialupWizardPage::shutdown() {
    return true;
}

//----------------------------------------------------------------------------
bool DialupWizardPage::activate() {
    return true;
}

//----------------------------------------------------------------------------
bool DialupWizardPage::deactivate() {
    return true;
}

//----------------------------------------------------------------------------