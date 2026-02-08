/* @file Окно настроек оборудования. */

#include "HardwareServiceWindow.h"

#include <QtGui/QShowEvent>

#include <SDK/Drivers/Components.h>

#include "Backend/HardwareManager.h"
#include "Backend/NetworkManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "DeviceSlot.h"
#include "EditorPane.h"
#include "HardwareWindow.h"
#include "MessageBox/MessageBox.h"

HardwareServiceWindow::HardwareServiceWindow(ServiceMenuBackend *aBackend, QWidget *aParent)
    : QFrame(aParent), ServiceWindowBase(aBackend), m_Window(new HardwareWindow(aBackend)) {
    setupUi(this);

    m_Window->setParent(this);
    m_Window->setSlotCreationMode(HardwareWindow::OpenEditorAfterCreation);

    wContainer->setCurrentWidget(wMainPage);

    wMainPage->setLayout(new QHBoxLayout);
    wMainPage->layout()->addWidget(m_Window);

    wEditorPage->setLayout(new QHBoxLayout);

    connect(m_Window, SIGNAL(detectionStarted()), SLOT(onDetectionStarted()));
    connect(m_Window, SIGNAL(detectionFinished()), SLOT(onDetectionFinished()));
    connect(m_Window, SIGNAL(applyingStarted()), SLOT(onApplyingStarted()));
    connect(m_Window, SIGNAL(applyingFinished()), SLOT(onApplyingFinished()));
    connect(m_Window,
            SIGNAL(editSlot(DeviceSlot *, EditorPane *)),
            SLOT(onEditSlot(DeviceSlot *, EditorPane *)));
    connect(m_Window, SIGNAL(removeSlot(DeviceSlot *)), SLOT(onRemoveSlot(DeviceSlot *)));
}

//------------------------------------------------------------------------
bool HardwareServiceWindow::activate() {
    m_Window->setConfiguration(m_Backend->getHardwareManager()->getConfiguration());

    return true;
}

//------------------------------------------------------------------------
bool HardwareServiceWindow::deactivate() {
    wContainer->setCurrentWidget(wMainPage);
    m_Backend->getHardwareManager()->setConfigurations(m_Window->getConfiguration().keys());
    m_Backend->saveConfiguration();

    return true;
}

//------------------------------------------------------------------------
bool HardwareServiceWindow::initialize() {
    if (!m_Window->initialize()) {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------
bool HardwareServiceWindow::shutdown() {
    m_Window->shutdown();

    return true;
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onDetectionStarted() {
    GUI::MessageBox::wait(tr("#detecting_devices"), true);
    GUI::MessageBox::subscribe(this);

    QVariantMap params;
    params[SDK::GUI::CMessageBox::ButtonType] = SDK::GUI::MessageBoxParams::Text;
    params[SDK::GUI::CMessageBox::ButtonText] = tr("#stop_search");

    GUI::MessageBox::update(params);
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onDetectionFinished() {
    m_Backend->getHardwareManager()->setConfigurations(m_Window->getConfiguration().keys());

    // Обновляем статусы найденных железок
    m_Backend->getHardwareManager()->updateStatuses();

    GUI::MessageBox::hide();
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onEditSlot(DeviceSlot *aSlot, EditorPane *aPane) {
    connect(aPane, SIGNAL(finished()), SLOT(onEditFinished()), Qt::UniqueConnection);

    wContainer->setCurrentWidget(wEditorPage);

    wEditorPage->layout()->addWidget(aPane->getWidget());

    if (aSlot->getType() == SDK::Driver::CComponents::Modem) {
        GUI::MessageBox::wait(tr("#closing_connection"));
        m_Backend->getNetworkManager()->closeConnection();
        GUI::MessageBox::hide();
    }
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onRemoveSlot(DeviceSlot *aSlot) {
    m_Window->removeDeviceSlot(aSlot, true);
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onEditFinished() {
    wContainer->setCurrentWidget(wMainPage);

    EditorPane *editor = qobject_cast<EditorPane *>(sender());
    if (editor) {
        wEditorPage->layout()->removeWidget(editor->getWidget());
        QString deviceType(editor->getSlot()->getType());

        if (editor->isChanged()) {
            editor->getSlot()->setParameterValues(editor->getParameterValues());
            m_Window->checkDeviceSlot(editor->getSlot());

            m_Backend->toLog(QString("UPDATE device: %1").arg(editor->getSlot()->getModel()));
        } else if (editor->getSlot()->getModel().isEmpty()) {
            m_Window->removeDeviceSlot(editor->getSlot());
        }

        if (deviceType == SDK::Driver::CComponents::Modem) {
            m_Backend->getNetworkManager()->openConnection();
        }
    }

    disconnect(sender(), SIGNAL(finished()), this, SLOT(onEditFinished()));
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onApplyingStarted() {
    GUI::MessageBox::wait(tr("#applying_configuration"));
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onApplyingFinished() {
    GUI::MessageBox::hide(true);

    // Для переинициализации свежедобавленного устройства. В противном случае не работает тест
    // купюроприемника.
    m_Window->setConfiguration(m_Backend->getHardwareManager()->getConfiguration());
}

//------------------------------------------------------------------------
void HardwareServiceWindow::onClicked(const QVariantMap & /*aParameters*/) {
    GUI::MessageBox::hide();
    GUI::MessageBox::wait(tr("#waiting_stop_search"));

    m_Window->abortDetection();

    m_Backend->toLog(QString("Button clicked: %1").arg(tr("#stop_search")));
}

//------------------------------------------------------------------------
