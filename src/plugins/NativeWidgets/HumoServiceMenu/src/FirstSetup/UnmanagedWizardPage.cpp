/* @file Окно настройки unmanaged. */

// Проект

// Project
#include "UnmanagedWizardPage.h"

UnmanagedWizardPage::UnmanagedWizardPage(HumoServiceBackend *aBackend, QWidget *aParent)
    : WizardPageBase(aBackend, aParent)
{
}

//----------------------------------------------------------------------------
bool UnmanagedWizardPage::initialize()
{
    return true;
}

//----------------------------------------------------------------------------
bool UnmanagedWizardPage::shutdown()
{
    return true;
}

//----------------------------------------------------------------------------
bool UnmanagedWizardPage::activate()
{
    return true;
}

//----------------------------------------------------------------------------
bool UnmanagedWizardPage::deactivate()
{
    return true;
}

//----------------------------------------------------------------------------