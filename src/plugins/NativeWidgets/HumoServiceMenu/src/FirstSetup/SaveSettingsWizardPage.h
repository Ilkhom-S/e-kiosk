/* @file Окно сохранения настроек. */

#pragma once

// Проект
#include "WizardPage.h"

class HumoServiceBackend;

//----------------------------------------------------------------------------
class SaveSettingsWizardPage : public WizardPageBase
{
    Q_OBJECT

  public:
    SaveSettingsWizardPage(HumoServiceBackend *aBackend, QWidget *aParent = 0);

    virtual bool initialize();
    virtual bool shutdown();

    virtual bool activate();
    virtual bool deactivate();
};

//----------------------------------------------------------------------------