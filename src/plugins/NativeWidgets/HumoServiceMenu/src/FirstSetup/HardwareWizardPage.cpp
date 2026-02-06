/* @file Окно настройки железа. */

#include "HardwareWizardPage.h"

#include <QtCore/QTimer>
#include <QtWidgets/QStackedLayout>

#include <SDK/Drivers/Components.h>

#include "Backend/HardwareManager.h"
#include "Backend/HumoServiceBackend.h"
#include "Backend/NetworkManager.h"
#include "GUI/DeviceSlot.h"
#include "GUI/HardwareWindow.h"
#include "GUI/MessageBox/MessageBox.h"

HardwareWizardPage::HardwareWizardPage(HumoServiceBackend *aBackend, QWidget *aParent)
    : WizardPageBase(aBackend, aParent) {
    QStackedLayout *layout = new QStackedLayout(this);

    setLayout(layout);

    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    // Создаём окно со списком устройств
    mHardwareWindow = new HardwareWindow(aBackend, this);
    mHardwareWindow->setSlotCreationMode(HardwareWindow::OpenEditorAfterCreation);

    // Создаём место для редактора устройств
    mEditorWindow = new QWidget(this);

    QHBoxLayout *editorLayout = new QHBoxLayout(mEditorWindow);

    mEditorWindow->setLayout(editorLayout);

    mEditorWindow->layout()->setSpacing(0);
    mEditorWindow->layout()->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(mHardwareWindow);
    layout->addWidget(mEditorWindow);

    layout->setCurrentWidget(mHardwareWindow);

    connect(mHardwareWindow, SIGNAL(detectionStarted()), SLOT(onDetectionStarted()));
    connect(mHardwareWindow, SIGNAL(detectionFinished()), SLOT(onDetectionFinished()));
    connect(mHardwareWindow, SIGNAL(applyingStarted()), SLOT(onApplyingStarted()));
    connect(mHardwareWindow, SIGNAL(applyingFinished()), SLOT(onApplyingFinished()));
    connect(mHardwareWindow,
            SIGNAL(editSlot(DeviceSlot *, EditorPane *)),
            SLOT(onEditSlot(DeviceSlot *, EditorPane *)));
    connect(mHardwareWindow, SIGNAL(removeSlot(DeviceSlot *)), SLOT(onRemoveSlot(DeviceSlot *)));
    connect(mHardwareWindow, SIGNAL(currentFormChanged(int)), SLOT(onCurrentFormChanged(int)));
}

//------------------------------------------------------------------------
bool HardwareWizardPage::initialize() {
    if (!mHardwareWindow->initialize()) {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------
bool HardwareWizardPage::shutdown() {
    mHardwareWindow->shutdown();

    return true;
}

//------------------------------------------------------------------------
bool HardwareWizardPage::activate() {
    mHardwareWindow->setConfiguration(mBackend->getHardwareManager()->getConfiguration());

    return true;
}

//------------------------------------------------------------------------
bool HardwareWizardPage::deactivate() {
    QStackedLayout *layout = qobject_cast<QStackedLayout *>(this->layout());
    if (layout) {
        layout->setCurrentWidget(mHardwareWindow);
    }

    mBackend->getHardwareManager()->setConfigurations(mHardwareWindow->getConfiguration().keys());

    return true;
}

//------------------------------------------------------------------------
void HardwareWizardPage::onDetectionStarted() {
    // TODO: Show waiting dialog
}

//------------------------------------------------------------------------
void HardwareWizardPage::onDetectionFinished() {
    // TODO: Hide waiting dialog
}

//------------------------------------------------------------------------
void HardwareWizardPage::onApplyingStarted() {
    // TODO: Show waiting dialog
}

//------------------------------------------------------------------------
void HardwareWizardPage::onApplyingFinished() {
    // TODO: Hide waiting dialog
}

//------------------------------------------------------------------------
void HardwareWizardPage::onEditSlot(DeviceSlot *aSlot, EditorPane *aPane) {
    QStackedLayout *layout = qobject_cast<QStackedLayout *>(this->layout());
    if (layout) {
        QHBoxLayout *editorLayout = qobject_cast<QHBoxLayout *>(mEditorWindow->layout());
        if (editorLayout) {
            editorLayout->addWidget(aPane->getWidget());
        }

        layout->setCurrentWidget(mEditorWindow);
    }
}

//------------------------------------------------------------------------
void HardwareWizardPage::onRemoveSlot(DeviceSlot *aSlot) {
    // TODO: Handle slot removal
}

//------------------------------------------------------------------------
void HardwareWizardPage::onEditFinished() {
    QStackedLayout *layout = qobject_cast<QStackedLayout *>(this->layout());
    if (layout) {
        layout->setCurrentWidget(mHardwareWindow);
    }
}

//------------------------------------------------------------------------
void HardwareWizardPage::onCurrentFormChanged(int aIndex) {
    // TODO: Handle form changes
}

//------------------------------------------------------------------------
void HardwareWizardPage::onClicked(const QVariantMap &aParameters) {
    // TODO: Handle clicks
}

//----------------------------------------------------------------------------