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
    m_HardwareWindow = new HardwareWindow(aBackend, this);
    m_HardwareWindow->setSlotCreationMode(HardwareWindow::OpenEditorAfterCreation);

    // Создаём место для редактора устройств
    m_EditorWindow = new QWidget(this);

    QHBoxLayout *editorLayout = new QHBoxLayout(m_EditorWindow);

    m_EditorWindow->setLayout(editorLayout);

    m_EditorWindow->layout()->setSpacing(0);
    m_EditorWindow->layout()->setContentsMargins(0, 0, 0, 0);

    layout->addWidget(m_HardwareWindow);
    layout->addWidget(m_EditorWindow);

    layout->setCurrentWidget(m_HardwareWindow);

    connect(m_HardwareWindow, SIGNAL(detectionStarted()), SLOT(onDetectionStarted()));
    connect(m_HardwareWindow, SIGNAL(detectionFinished()), SLOT(onDetectionFinished()));
    connect(m_HardwareWindow, SIGNAL(applyingStarted()), SLOT(onApplyingStarted()));
    connect(m_HardwareWindow, SIGNAL(applyingFinished()), SLOT(onApplyingFinished()));
    connect(m_HardwareWindow,
            SIGNAL(editSlot(DeviceSlot *, EditorPane *)),
            SLOT(onEditSlot(DeviceSlot *, EditorPane *)));
    connect(m_HardwareWindow, SIGNAL(removeSlot(DeviceSlot *)), SLOT(onRemoveSlot(DeviceSlot *)));
    connect(m_HardwareWindow, SIGNAL(currentForm_Changed(int)), SLOT(onCurrentForm_Changed(int)));
}

//------------------------------------------------------------------------
bool HardwareWizardPage::initialize() {
    if (!m_HardwareWindow->initialize()) {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------
bool HardwareWizardPage::shutdown() {
    m_HardwareWindow->shutdown();

    return true;
}

//------------------------------------------------------------------------
bool HardwareWizardPage::activate() {
    m_HardwareWindow->setConfiguration(m_Backend->getHardwareManager()->getConfiguration());

    return true;
}

//------------------------------------------------------------------------
bool HardwareWizardPage::deactivate() {
    QStackedLayout *layout = qobject_cast<QStackedLayout *>(this->layout());
    if (layout) {
        layout->setCurrentWidget(m_HardwareWindow);
    }

    m_Backend->getHardwareManager()->setConfigurations(m_HardwareWindow->getConfiguration().keys());

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
        QHBoxLayout *editorLayout = qobject_cast<QHBoxLayout *>(m_EditorWindow->layout());
        if (editorLayout) {
            editorLayout->addWidget(aPane->getWidget());
        }

        layout->setCurrentWidget(m_EditorWindow);
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
        layout->setCurrentWidget(m_HardwareWindow);
    }
}

//------------------------------------------------------------------------
void HardwareWizardPage::onCurrentForm_Changed(int aIndex) {
    // TODO: Handle form changes
}

//------------------------------------------------------------------------
void HardwareWizardPage::onClicked(const QVariantMap &aParameters) {
    // TODO: Handle clicks
}

//----------------------------------------------------------------------------