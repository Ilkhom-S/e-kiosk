/* @file Окно выбора типа сети. */

#pragma once

#include "WizardPage.h"
#include "ui_NetworkWizardPage.h"

//---------------------------------------------------------------------------
class NetworkWizardPage : public WizardPageBase, protected Ui::NetworkWizardPage {
    Q_OBJECT

public:
    NetworkWizardPage(ServiceMenuBackend *aBackend, QWidget *aParent = 0);

    virtual bool initialize();
    virtual bool shutdown();

    virtual bool activate();
    virtual bool deactivate();

private slots:
    void onChooseDialup();
    void onChooseUnmanaged();
};

//---------------------------------------------------------------------------
