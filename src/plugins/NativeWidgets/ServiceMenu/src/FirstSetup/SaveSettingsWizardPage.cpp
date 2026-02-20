/* @file Окно сохранения настроек. */

#include "SaveSettingsWizardPage.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFuture>

#include "Backend/ServiceMenuBackend.h"
#include "GUI/ServiceTags.h"

//----------------------------------------------------------------------------
SaveSettingsWizardPage::SaveSettingsWizardPage(ServiceMenuBackend *aBackend, QWidget *aParent)
    : WizardPageBase(aBackend, aParent) {
    setupUi(this);
}

//------------------------------------------------------------------------
bool SaveSettingsWizardPage::initialize() {
    return true;
}

//------------------------------------------------------------------------
bool SaveSettingsWizardPage::shutdown() {
    return true;
}

//------------------------------------------------------------------------
bool SaveSettingsWizardPage::activate() {
    pbProgress->setRange(0, 0); // Indeterminate progress
    onSave();

    return true;
}

//------------------------------------------------------------------------
bool SaveSettingsWizardPage::deactivate() {
    return true;
}

//----------------------------------------------------------------------------
void SaveSettingsWizardPage::onSave() {
    lbStatus->setText(tr("#saving_configs"));

    // TODO Надо получать описание ошибки
    if (!m_Backend->saveConfiguration()) {
        showError(tr("#when_saving_configs"), tr("#save_configuration_error"));
        return;
    }

    lbStatus->setText(tr("#saved_successfully"));
    pbProgress->setRange(0, 100);
    pbProgress->setValue(100);

    // Завершаем сценарий после успешного сохранения
    QTimer::singleShot(2000, this, SLOT(onFinish()));
}

//----------------------------------------------------------------------------
void SaveSettingsWizardPage::onFinish() {
    QVariantMap parameters;
    parameters["signal"] = "exit";

    // Завершаем сценарий.
    m_Backend->sendEvent(SDK::PaymentProcessor::EEventType::UpdateScenario, parameters);

    m_Backend->sendEvent(SDK::PaymentProcessor::EEventType::CloseApplication);
}

//----------------------------------------------------------------------------
void SaveSettingsWizardPage::showError(const QString &aContext, const QString &aError) {
    lbStatus->setText(aContext + "\n" + aError);
    pbProgress->setRange(0, 100);
    pbProgress->setValue(0);
}

//----------------------------------------------------------------------------
