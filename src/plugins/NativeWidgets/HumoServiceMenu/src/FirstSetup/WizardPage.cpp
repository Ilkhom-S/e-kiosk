/* @file Окно визарда. */

// Проект

// Project
#include "WizardPage.h"

WizardPageBase::WizardPageBase(HumoServiceBackend *aBackend, QWidget *aParent)
    : ServiceWindowBase(aBackend), QFrame(aParent)
{
}

//------------------------------------------------------------------------