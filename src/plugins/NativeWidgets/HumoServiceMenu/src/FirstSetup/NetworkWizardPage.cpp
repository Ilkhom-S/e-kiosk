/* @file Окно настройки сети. */

#include "NetworkWizardPage.h"

NetworkWizardPage::NetworkWizardPage(HumoServiceBackend *aBackend, QWidget *aParent)
    : WizardPageBase(aBackend, aParent) {}

//----------------------------------------------------------------------------
bool NetworkWizardPage::initialize() {
    return true;
}

//----------------------------------------------------------------------------
bool NetworkWizardPage::shutdown() {
    return true;
}

//----------------------------------------------------------------------------
bool NetworkWizardPage::activate() {
    return true;
}

//----------------------------------------------------------------------------
bool NetworkWizardPage::deactivate() {
    return true;
}

//----------------------------------------------------------------------------