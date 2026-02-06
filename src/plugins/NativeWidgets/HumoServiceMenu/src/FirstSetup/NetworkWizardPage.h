/* @file Окно настройки сети. */

#pragma once

#include "WizardPage.h"

class HumoServiceBackend;

//----------------------------------------------------------------------------
class NetworkWizardPage : public WizardPageBase {
    Q_OBJECT

public:
    NetworkWizardPage(HumoServiceBackend *aBackend, QWidget *aParent = 0);

    virtual bool initialize();
    virtual bool shutdown();

    virtual bool activate();
    virtual bool deactivate();
};

//----------------------------------------------------------------------------