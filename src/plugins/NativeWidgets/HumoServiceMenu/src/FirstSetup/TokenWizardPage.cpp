/* @file Окно настройки token. */

// Проект

// Project
#include "TokenWizardPage.h"

TokenWizardPage::TokenWizardPage(HumoServiceBackend *aBackend, QWidget *aParent) : WizardPageBase(aBackend, aParent)
{
}

//----------------------------------------------------------------------------
bool TokenWizardPage::initialize()
{
    return true;
}

//----------------------------------------------------------------------------
bool TokenWizardPage::shutdown()
{
    return true;
}

//----------------------------------------------------------------------------
bool TokenWizardPage::activate()
{
    return true;
}

//----------------------------------------------------------------------------
bool TokenWizardPage::deactivate()
{
    return true;
}

//----------------------------------------------------------------------------