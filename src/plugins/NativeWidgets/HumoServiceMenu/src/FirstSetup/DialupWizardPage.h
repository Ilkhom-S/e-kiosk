/* @file Окно настройки dialup. */

#pragma once

// Проект
#include "WizardPage.h"

class HumoServiceBackend;

//----------------------------------------------------------------------------
class DialupWizardPage : public WizardPageBase
{
    Q_OBJECT

  public:
    DialupWizardPage(HumoServiceBackend *aBackend, QWidget *aParent = 0);

    virtual bool initialize();
    virtual bool shutdown();

    virtual bool activate();
    virtual bool deactivate();
};

//----------------------------------------------------------------------------