/* @file Окно приветствия. */

#include "WelcomeWizardPage.h"

#include "WizardContext.h"

//----------------------------------------------------------------------------
WelcomeWizardPage::WelcomeWizardPage(ServiceMenuBackend *aBackend, QWidget *aParent)
    : WizardPageBase(aBackend, aParent) {
    setupUi(this);

    // Загружаем SVG логотип программно, т.к. Qt Designer не корректно обрабатывает SVG в <pixmap>
    lbLogo->setPixmap(QPixmap(":/Images/humo_logo_midnight.svg"));

    connect(btnSetup, SIGNAL(clicked()), SLOT(onRunSetup()));
}

//---------------------------------------------------------------------------
bool WelcomeWizardPage::initialize() {
    return true;
}

//---------------------------------------------------------------------------
bool WelcomeWizardPage::shutdown() {
    return true;
}

//---------------------------------------------------------------------------
bool WelcomeWizardPage::activate() {
    return true;
}

//---------------------------------------------------------------------------
bool WelcomeWizardPage::deactivate() {
    return true;
}

//---------------------------------------------------------------------------
void WelcomeWizardPage::onRunSetup() {
    emit pageEvent(CWizardContext::RunSetup, true);
}

//---------------------------------------------------------------------------
