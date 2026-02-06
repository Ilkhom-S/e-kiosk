/* @file Окно визарда. */

#include "WizardPage.h"

//------------------------------------------------------------------------
WizardPageBase::WizardPageBase(ServiceMenuBackend *aBackend, QWidget *aParent)
    : ServiceWindowBase(aBackend), QFrame(aParent) {}

//------------------------------------------------------------------------
