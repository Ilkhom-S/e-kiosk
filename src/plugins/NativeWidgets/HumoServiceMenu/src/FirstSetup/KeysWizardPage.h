/* @file Окно настройки keys. */

#pragma once

// Проект
#include "WizardPage.h"

class HumoServiceBackend;

//----------------------------------------------------------------------------
class KeysWizardPage : public WizardPageBase
{
    Q_OBJECT

  public:
    KeysWizardPage(HumoServiceBackend *aBackend, QWidget *aParent = 0);

    virtual bool initialize();
    virtual bool shutdown();

    virtual bool activate();
    virtual bool deactivate();
};

//----------------------------------------------------------------------------