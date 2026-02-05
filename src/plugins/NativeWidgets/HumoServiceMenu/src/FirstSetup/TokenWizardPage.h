/* @file Окно настройки token. */

#pragma once

// Проект
#include "WizardPage.h"

class HumoServiceBackend;

//----------------------------------------------------------------------------
class TokenWizardPage : public WizardPageBase
{
    Q_OBJECT

  public:
    TokenWizardPage(HumoServiceBackend *aBackend, QWidget *aParent = 0);

    virtual bool initialize();
    virtual bool shutdown();

    virtual bool activate();
    virtual bool deactivate();
};

//----------------------------------------------------------------------------