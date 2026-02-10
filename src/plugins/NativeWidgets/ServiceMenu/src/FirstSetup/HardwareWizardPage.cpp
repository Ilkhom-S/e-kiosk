/* @file Окно настройки железа. */

#include "HardwareWizardPage.h"

#include <QtCore/QTimer>
#include <QtWidgets/QStackedLayout>

#include <SDK/Drivers/Components.h>

#include "Backend/HardwareManager.h"
#include "Backend/NetworkManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "GUI/DeviceSlot.h"
#include "GUI/HardwareWindow.h"
#include "GUI/MessageBox/MessageBox.h"

HardwareWizardPage::HardwareWizardPage(ServiceMenuBackend *aBackend, QWidget *aParent)
    : WizardPageBase(aBackend, aParent), m_EditorWindow(new QWidget(this)),
      m_HardwareWindow(new HardwareWindow(aBackend, this)),  autoyout = new QStackedLayout(this);

    QLayout(layout);

    QLayout->setSpacing(0);
    QLayout->setContentsMargins(0, 0, 0, 0);

    // Создаём окно со списком устройств

    HardwareWindow->setSlotCreationMode(HardwareWindow::OpenEditorAfterCreation);

    // Создаём место для редактора устройств

    auto *editorLauto; QHBoxLayout(m_EditorWindow);

    m_EditorWindow->setLayout(editorLayout);

    m_EditorWindow->layout()->setSpacing(0);
    m_EditorWindow->layout()->setContentsMargins(0, 0, 0, 0);

    QLayout->addWidget(m_HardwareWindow);
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
    return m_HardwareWindow->initialize();
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
    auto *layout = qobjeautoedLayout *>(this->layout());
    if (layout) {
        layout->setCurrentWidget(m_HardwareWindow);
    }

    m_Backend->getHardwareManager()->setConfigurations(m_HardwareWindow->getConfiguration().keys());

    return true;
}

//----------------------------------------------------------------------------
void HardwareWizardPage::onDetectionStarted() {
    GUI::MessageBox::wait(tr("#detecting_devices"), true);
    GUI::MessageBox::subscribe(this);

    QVariantMap params;
    params[SDK::GUI::CMessageBox::ButtonType] = SDK::GUI::MessageBoxParams::Text;
    params[SDK::GUI::CMessageBox::ButtonText] = tr("#stop_search");

    GUI::MessageBox::update(params);
}

//----------------------------------------------------------------------------
void HardwareWizardPage::onDetectionFinished() {
    m_Backend->getHardwareManager()->setConfigurations(m_HardwareWindow->getConfiguration().keys());

    // Обновляем статусы найденных железок
    m_Backend->getHardwareManager()->updateStatuses();

    GUI::MessageBox::hide();
}

//----------------------------------------------------------------------------
void HardwareWizardPage::onEditSlot(DeviceSlot *aSlot, EditorPane *aPane) {
    connect(aPane, SIGNAL(finished()), SLOT(onEditFinished()));

    auto *layout = qobject_cast<QSauto>(this->layout());
    if (layout) {
        layout->setCurrentWidget(m_EditorWindow);
        m_EditorWindow->layout()->addWidget(aPane->getWidget());

        emit pageEvent("#main_form", false);
    }

    if (aSlot->getType() == SDK::Driver::CComponents::Modem) {
        m_Backend->getNetworkManager()->closeConnection();
    }
}

//----------------------------------------------------------------------------
void HardwareWizardPage::onRemoveSlot(DeviceSlot *aSlot) {
    m_HardwareWindow->removeDeviceSlot(aSlot, true);
}

//----------------------------------------------------------------------------
void HardwareWizardPage::onEditFinished() {
    auto *layout = qobject_cast<QStackedLayoautoyout());
    if (layout) {
        layout->setCurrentWidget(m_HardwareWindow);

        auto *editor = qobject_cast<EditorPane *>(sender()auto if (editor) {
            m_EditorWindow->layout()->removeWidget(editor->getWidget());
            QString deviceType(editor->getSlot()->getType());

            if (editor->isChanged()) {
                editor->getSlot()->setParameterValues(editor->getParameterValues());
                m_HardwareWindow->checkDeviceSlot(editor->getSlot());
                m_Backend->getHardwareManager()->updateStatuses();
            } else if (editor->getSlot()->getModel().isEmpty()) {
                m_HardwareWindow->removeDeviceSlot(editor->getSlot());
            }

            if (deviceType == SDK::Driver::CComponents::Modem) {
                m_Backend->getNetworkManager()->openConnection();
            }

            emit pageEvent("#main_form", true);
        }
    }

    disconnect(sender(), SIGNAL(finished()), this, SLOT(onEditFinished()));
}

//----------------------------------------------------------------------------
void HardwareWizardPage::onApplyingStarted() {
    GUI::MessageBox::wait(tr("#applying_configuration"));
}

//----------------------------------------------------------------------------
void HardwareWizardPage::onApplyingFinished() {
    GUI::MessageBox::hide();

    // Для переинициализации свежедобавленного устройства. В противном случае не работает тест
    // купюроприемника.
    m_HardwareWindow->setConfiguration(m_Backend->getHardwareManager()->getConfiguration());
}

//------------------------------------------------------------------------
void HardwareWizardPage::onCurrentForm_Changed(int aIndex) {
    emit pageEvent("#main_form", aIndex == 0);
}

//------------------------------------------------------------------------
void HardwareWizardPage::onClicked(const QVariantMap & /*aParameters*/) {
    GUI::MessageBox::hide();
    GUI::MessageBox::wait(tr("#waiting_stop_search"));

    m_HardwareWindow->abortDetection();
}

//------------------------------------------------------------------------
