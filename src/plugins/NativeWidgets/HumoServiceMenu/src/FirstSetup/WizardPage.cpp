/* @file Окно визарда. */

#include "WizardPage.h"

WizardPageBase::WizardPageBase(HumoServiceBackend *aBackend, QWidget *aParent)
    : ServiceWindowBase(aBackend), QFrame(aParent) {}

//------------------------------------------------------------------------