/* @file Реализация компоненты для редактирования профилей устройств. */

#include "DeviceSlot.h"

#include <QtCore/QBuffer>
#include <QtCore/QTimer>
#include <QtCore/QUrl>

#include <SDK/Drivers/Components.h>
#include <SDK/Drivers/IDevice.h>
#include <SDK/PaymentProcessor/IDeviceTest.h>

#include "Backend/HardwareManager.h"
#include "Backend/HumoServiceBackend.h"
#include "DeviceTests/BillAcceptorTest.h"
#include "DeviceTests/CoinAcceptorTest.h"
#include "DeviceTests/DispenserTest.h"
#include "DeviceTests/GenericDeviceTest.h"
#include "DeviceTests/HIDTest.h"
#include "DeviceTests/PrinterTest.h"
#include "MessageBox/MessageBox.h"

DeviceSlot::DeviceSlot(HumoServiceBackend *aBackend,
                       const QString &aConfigurationName,
                       bool aIsUserSlot,
                       const QString &aType)
    : m_Backend(aBackend), m_ConfigurationName(aConfigurationName), m_IsUserSlot(aIsUserSlot),
      m_Type(aType) {
    updateConfigurationName(m_ConfigurationName);
    QTimer::singleShot(0, this, SLOT(onRepaint()));
}

//------------------------------------------------------------------------
DeviceSlot::~DeviceSlot() {
    if (m_Widget) {
        m_Widget->deleteLater();
    }
}

//------------------------------------------------------------------------
QWidget *DeviceSlot::createWidget() {
    QWidget *widget = new QWidget();

    ui.setupUi(widget);

    connect(ui.btnChange, SIGNAL(clicked()), SLOT(onClick()));
    connect(ui.btnDelete, SIGNAL(clicked()), SLOT(onRemove()));
    connect(ui.btnRunTest, SIGNAL(clicked()), this, SLOT(onDeviceRunTest()));

    return widget;
}

//------------------------------------------------------------------------
QWidget *DeviceSlot::getWidget() {
    if (!m_Widget) {
        m_Widget = createWidget();
    }

    return m_Widget;
}

//------------------------------------------------------------------------
const QString &DeviceSlot::getType() const {
    return m_Type;
}

//------------------------------------------------------------------------
QString DeviceSlot::getModel() const {
    return m_ParameterValues.contains("model_name") ? m_ParameterValues["model_name"].toString()
                                                    : QString();
}

//------------------------------------------------------------------------
bool DeviceSlot::isUserSlot() const {
    return m_IsUserSlot;
}

//------------------------------------------------------------------------
void DeviceSlot::setParameterValues(QVariantMap aValues) {
    // Если сменили модель, устройство надо пересоздать
    if (!m_ParameterValues["model_name"].isNull() &&
        m_ParameterValues["model_name"] != aValues["model_name"]) {
        m_Backend->getHardwareManager()->releaseDevice(m_ConfigurationName);
        m_ConfigurationName.clear();
    }

    m_ParameterValues = aValues;

    QTimer::singleShot(0, this, SLOT(onRepaint()));
}

//------------------------------------------------------------------------
const QVariantMap &DeviceSlot::getParameterValues() const {
    return m_ParameterValues;
}

//------------------------------------------------------------------------
HumoServiceBackend *DeviceSlot::getBackend() const {
    return m_Backend;
}

//------------------------------------------------------------------------
QString DeviceSlot::getConfigurationName() const {
    return m_ConfigurationName;
}

//------------------------------------------------------------------------
void DeviceSlot::updateConfigurationName(const QString &aConfigurationName) {
    if (aConfigurationName.isEmpty()) {
        return;
    }

    m_ConfigurationName = aConfigurationName;
    m_Type = m_ConfigurationName.section(".", 2, 2);

    m_Device = m_Backend->getHardwareManager()->getDevice(m_ConfigurationName);

    if (m_Device) {
        if (m_Type == SDK::Driver::CComponents::BillAcceptor) {
            m_DeviceTest =
                QSharedPointer<SDK::PaymentProcessor::IDeviceTest>(new BillAcceptorTest(m_Device));
        } else if (m_Type == SDK::Driver::CComponents::CoinAcceptor) {
            m_DeviceTest =
                QSharedPointer<SDK::PaymentProcessor::IDeviceTest>(new CoinAcceptorTest(m_Device));
        } else if (SDK::Driver::CComponents::isPrinter(m_Type)) {
            m_DeviceTest = QSharedPointer<SDK::PaymentProcessor::IDeviceTest>(
                new PrinterTest(m_Device, m_Backend->getCore()));
        } else if (m_Type == SDK::Driver::CComponents::Scanner ||
                   m_Type == SDK::Driver::CComponents::Camera) {
            m_DeviceTest = QSharedPointer<SDK::PaymentProcessor::IDeviceTest>(
                new HIDTest(m_Device, m_ConfigurationName));
        } else if (m_Type == SDK::Driver::CComponents::Dispenser) {
            m_DeviceTest = QSharedPointer<SDK::PaymentProcessor::IDeviceTest>(
                new DispenserTest(m_Device, m_ConfigurationName, m_Backend->getCore()));
        } else {
            m_DeviceTest =
                QSharedPointer<SDK::PaymentProcessor::IDeviceTest>(new GenericDeviceTest(m_Device));
        }
    }
}

//------------------------------------------------------------------------
void DeviceSlot::updateDeviceStatus(const QString &aNewStatus,
                                    const QString &aStatusColor,
                                    SDK::Driver::EWarningLevel::Enum /*aLevel*/) {
    ui.lbStatus->setText(aNewStatus);
    ui.lineStatus->setStyleSheet(ui.lineStatus->styleSheet() + ";color:" + aStatusColor +
                                 ";background-color: " + aStatusColor);

    if (!m_DeviceTest.isNull()) {
        ui.btnRunTest->setEnabled(m_DeviceTest->isReady());
    }
}

//------------------------------------------------------------------------------
void DeviceSlot::onDeviceRunTest() {
    connect(m_DeviceTest.data(),
            SIGNAL(result(const QString &, const QVariant &)),
            this,
            SLOT(onTestResult(const QString &, const QVariant &)));

    if (m_DeviceTest->hasResult()) {
        GUI::MessageBox::subscribe(this);
    }

    foreach (auto test, m_DeviceTest->getTestNames()) {
        if (m_DeviceTest->hasResult()) {
            GUI::MessageBox::info(test.second);
        }

        m_DeviceTest->run(test.first);
    }

    m_Backend->toLog(QString("Button clicked: %1; Device: %2:%3.")
                         .arg(qobject_cast<QAbstractButton *>(sender())->text())
                         .arg(m_Type)
                         .arg(getModel()));
}

//------------------------------------------------------------------------------
void DeviceSlot::onTestResult(const QString &aTestName, const QVariant &aTestResult) {
    Q_UNUSED(aTestName);

    QVariantMap params;

    switch (aTestResult.typeId()) {
    case QMetaType::QImage: {
        QBuffer buffer;
        if (aTestResult.value<QImage>().save(&buffer, "jpg", 70)) {
            QByteArray data = buffer.data().toBase64();
            data.insert(0, "data:image/jpeg;base64,");
            data.squeeze();

            params[SDK::GUI::CMessageBox::Image] = QString::from_Latin1(data);
        }
    } break;

    default:
        params[SDK::GUI::CMessageBox::TextMessageExt] = aTestResult;
    }

    GUI::MessageBox::update(params);
}

//------------------------------------------------------------------------------
void DeviceSlot::onClicked(const QVariantMap & /*aParameters*/) {
    m_DeviceTest->stop();
    disconnect(m_DeviceTest.data(),
               SIGNAL(result(const QString &, const QVariant &)),
               this,
               SLOT(onTestResult(const QString &, const QVariant &)));

    GUI::MessageBox::hide();
}

//------------------------------------------------------------------------
void DeviceSlot::onRepaint() {
    QString model = getModel();
    QString title;

    if (!model.isEmpty()) {
        title = model;
    } else {
        title = QCoreApplication::translate(
            "Hardware::CommonParameters",
            QT_TRANSLATE_NOOP("Hardware::CommonParameters", "#unknown_model"));
    }

    // Для устройств вида Класс.Модель выкусываем название класса
    ui.btnChange->setText(QString("%1: %2")
                              .arg(QCoreApplication::translate(
                                  "Hardware::Types", m_Type.section(".", 0, 0).toLatin1()))
                              .arg(title));
}

//------------------------------------------------------------------------
void DeviceSlot::onClick() {
    emit edit();
}

//------------------------------------------------------------------------
void DeviceSlot::onRemove() {
    m_Backend->toLog(QString("Button clicked: %1; Device: %2:%3.")
                         .arg(qobject_cast<QAbstractButton *>(sender())->text())
                         .arg(m_Type)
                         .arg(getModel()));

    emit remove();
}

//------------------------------------------------------------------------
