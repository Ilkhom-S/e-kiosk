/* @file Виджет, отображающий состояние устройства */

#include "DeviceStatusWindow.h"

#include "Backend/HardwareManager.h"
#include "Backend/HumoServiceBackend.h"

DeviceStatusWindow::DeviceStatusWindow(HumoServiceBackend *aBackend,
                                       const QString &aConfigurationName,
                                       QWidget *aParent)
    : DeviceSlot(aBackend, aConfigurationName) {
    setParent(aParent);
}

//------------------------------------------------------------------------------
DeviceStatusWindow::~DeviceStatusWindow() {}

//------------------------------------------------------------------------
QWidget *DeviceStatusWindow::createWidget() {
    QFrame *widget = new QFrame(dynamic_cast<QWidget *>(parent()));

    ui.setupUi(widget);

    ui.lblDeviceType->setText(m_Type);

    QVariantMap deviceParams(
        m_Backend->getHardwareManager()->getConfiguration()[m_ConfigurationName].toMap());
    ui.lblDeviceModel->setText(deviceParams["model_name"].toString());

    ui.btnRunTest->setEnabled(m_DeviceTest ? m_DeviceTest->isReady() : false);

    connect(ui.btnRunTest, SIGNAL(clicked()), this, SLOT(onDeviceRunTest()));

    return widget;
}

//------------------------------------------------------------------------------
void DeviceStatusWindow::onRepaint() {
    // no need repaint
}

//------------------------------------------------------------------------------
void DeviceStatusWindow::updateDeviceStatus(const QString &aNewStatus,
                                            const QString &aStatusColor,
                                            SDK::Driver::EWarningLevel::Enum /*aLevel*/) {
    ui.lblDeviceStatus->setText(aNewStatus);
    ui.lblDeviceModel->setStyleSheet("color:" + aStatusColor + ";");
    ui.lblDeviceStatus->setStyleSheet("color:" + aStatusColor + ";");
    ui.lblDeviceType->setStyleSheet("color:" + aStatusColor + ";");

    if (!m_DeviceTest.isNull()) {
        ui.btnRunTest->setEnabled(m_DeviceTest->isReady());
    }
}

//------------------------------------------------------------------------------
