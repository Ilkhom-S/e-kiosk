/* @file Окно настройки unmanaged. */

#pragma once

#include "WizardPage.h"

class HumoServiceBackend;

//----------------------------------------------------------------------------
class UnmanagedWizardPage : public WizardPageBase {
    Q_OBJECT

public:
    UnmanagedWizardPage(HumoServiceBackend *aBackend, QWidget *aParent = 0);

    virtual bool initialize();
    virtual bool shutdown();

    virtual bool activate();
    virtual bool deactivate();
};

//----------------------------------------------------------------------------