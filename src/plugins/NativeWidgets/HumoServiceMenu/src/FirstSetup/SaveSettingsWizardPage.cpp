/* @file Окно сохранения настроек. */

#include "SaveSettingsWizardPage.h"

SaveSettingsWizardPage::SaveSettingsWizardPage(HumoServiceBackend *aBackend, QWidget *aParent)
    : WizardPageBase(aBackend, aParent) {}

//----------------------------------------------------------------------------
bool SaveSettingsWizardPage::initialize() {
    return true;
}

//----------------------------------------------------------------------------
bool SaveSettingsWizardPage::shutdown() {
    return true;
}

//----------------------------------------------------------------------------
bool SaveSettingsWizardPage::activate() {
    return true;
}

//----------------------------------------------------------------------------
bool SaveSettingsWizardPage::deactivate() {
    return true;
}

//----------------------------------------------------------------------------