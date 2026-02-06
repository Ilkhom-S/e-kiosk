/* @file Окно приветствия. */

#pragma once

#include "WizardPage.h"
#include "ui_WelcomeWizardPage.h"

class HumoServiceBackend;

//---------------------------------------------------------------------------
class WelcomeWizardPage : public WizardPageBase, protected Ui::WelcomeWizardPage {
    Q_OBJECT

public:
    WelcomeWizardPage(HumoServiceBackend *aBackend, QWidget *aParent = 0);

    virtual bool initialize();
    virtual bool shutdown();

    virtual bool activate();
    virtual bool deactivate();

private slots:
    void onRunSetup();
};

//---------------------------------------------------------------------------