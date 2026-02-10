/* @file Виджет авто инкассации */

#include "AutoEncashmentWindow.h"

#include <QtCore/QDateTime>
#include <QtWidgets/QInputDialog>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>

#include "Backend/HardwareManager.h"
#include "Backend/HumoServiceBackend.h"
#include "Backend/PaymentManager.h"
#include "GUI/EncashmentWindow.h"
#include "GUI/InputBox.h"
#include "GUI/MessageBox/MessageBox.h"
#include "GUI/ServiceTags.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
namespace CAutoEncashmentWindow {
// Максимальное время бездействия в окне авто инкассации.
const int AutoEncashmentIdleTimeout = 2 * 60 * 1000; // 2 минуты.

// Интервал обновления текущей даты/времени на экране.
const int DateTimeRefreshInterval = 60 * 1000; // 1 минута
} // namespace CAutoEncashmentWindow

//---------------------------------------------------------------------------
AutoEncashmentWindow::AutoEncashmentWindow(HumoServiceBackend *aBackend, QWidget *aParent)
    : EncashmentWindow(aBackend, aParent) {
    ui.setupUi(this);

    ui.lbCurrentDate->setText(QDateTime::currentDateTime().toString("dd/MM/yyyy, hh:mm"));

    connect(ui.btnTestPrinter, SIGNAL(clicked()), SLOT(onTestPrinter()));
    connect(ui.btnEncashment, SIGNAL(clicked()), SLOT(onEncashment()));
    connect(ui.btnEncashmentAndZReport, SIGNAL(clicked()), SLOT(onEncashmentAndZReport()));
    connect(ui.btnEnterServiceMenu, SIGNAL(clicked()), SLOT(onEnterServiceMenu()));
    connect(ui.btnShowHistory, SIGNAL(clicked()), SLOT(onShowHistory()));
    connect(ui.btnExit, SIGNAL(clicked()), SLOT(onExit()));

    connect(ui.stackedWidget, SIGNAL(currentChanged(int)), this, SLOT(onPanelChanged(int)));
    connect(ui.btnBack, SIGNAL(clicked()), SLOT(onBack()));

    m_IdleTimer.setInterval(CAutoEncashmentWindow::AutoEncashmentIdleTimeout);
    connect(&m_IdleTimer, SIGNAL(timeout()), SLOT(onIdleTimeout()));

    m_DateTimeTimer.setInterval(CAutoEncashmentWindow::DateTimeRefreshInterval);
    connect(&m_DateTimeTimer, SIGNAL(timeout()), SLOT(onDateTimeRefresh()));

    PaymentManager *paymentManager = m_Backend->getPaymentManager();
    connect(
        paymentManager, SIGNAL(receiptPrinted(qint64, bool)), SLOT(onReceiptPrinted(qint64, bool)));

    m_HistoryWindow = new EncashmentHistoryWindow(aBackend, this);
    ui.verticalLayoutHistory->insertWidget(0, m_HistoryWindow, 1);
}

//---------------------------------------------------------------------------
AutoEncashmentWindow::~AutoEncashmentWindow() = default;

//---------------------------------------------------------------------------
bool AutoEncashmentWindow::initialize() {
    m_Backend->getTerminalInfo(m_TerminalInfo);

    ui.lbTerminalNumber->setText(tr("#terminal_number") +
                                 m_TerminalInfo[CServiceTags::TerminalNumber].toString());
    ui.lbVersion->setText(tr("#software_version") +
                          m_TerminalInfo[CServiceTags::SoftwareVersion].toString());
    ui.stackedWidget->setCurrentIndex(0);

    m_IdleTimer.start();
    m_DateTimeTimer.start();

    activate();

    return true;
}

//---------------------------------------------------------------------------
bool AutoEncashmentWindow::shutdown() {
    deactivate();

    return true;
}

//---------------------------------------------------------------------------
bool AutoEncashmentWindow::activate() {
    connect(
        m_Backend->getHardwareManager(),
        SIGNAL(deviceStatusChanged(
            const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)),
        this,
        SLOT(onDeviceStatusChanged(
            const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)));

    updateUI();

    return EncashmentWindow::activate();
}

//---------------------------------------------------------------------------
bool AutoEncashmentWindow::deactivate() {
    disconnect(
        m_Backend->getHardwareManager(),
        SIGNAL(deviceStatusChanged(
            const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)),
        this,
        SLOT(onDeviceStatusChanged(
            const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)));

    return EncashmentWindow::deactivate();
}

//------------------------------------------------------------------------
void AutoEncashmentWindow::onDeviceStatusChanged(const QString &aConfigName,
                                                 const QString &aStatusString,
                                                 const QString &aStatusColor,
                                                 SDK::Driver::EWarningLevel::Enum aLevel) {
    Q_UNUSED(aConfigName);
    Q_UNUSED(aStatusString);
    Q_UNUSED(aStatusColor);
    Q_UNUSED(aLevel);

    updateUI();
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::updateUI() {
    ui.btnEncashment->setEnabled(true);

    ui.btnEncashmentAndZReport->setEnabled(
        m_Backend->getHardwareManager()->isFiscalPrinterPresent(true));

    m_Backend->getPaymentManager()->useHardwareFiscalPrinter(
        m_Backend->getHardwareManager()->isFiscalPrinterPresent(false));
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onEncashment() {
    m_EncashmentWithZReport = false;

    doEncashment();
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onTestPrinter() {
    auto *paymentManager = m_Backend->getPaymentManager();
    bool isPrinterOK = paymentManager->canPrint(PPSDK::CReceiptType::Encashment);

    if (!isPrinterOK) {
        GUI::MessageBox::warning(tr("#printer_failed"));

        return;
    }

    paymentManager->printTestPage();
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onEncashmentAndZReport() {
    m_EncashmentWithZReport = true;

    doEncashment();
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onEnterServiceMenu() {
    m_IdleTimer.stop();

    QVariantMap params;
    params["name"] = "service_menu";
    m_Backend->sendEvent(SDK::PaymentProcessor::EEventType::StartScenario, params);

    deactivate();
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onShowHistory() {
    ui.stackedWidget->setCurrentIndex(1);
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onBack() {
    ui.stackedWidget->setCurrentIndex(0);
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onExit() {
    m_IdleTimer.stop();

    QVariantMap params;
    params["signal"] = "close";
    m_Backend->sendEvent(SDK::PaymentProcessor::EEventType::UpdateScenario, params);

    deactivate();
}

//------------------------------------------------------------------------
void AutoEncashmentWindow::onDateTimeRefresh() {
    ui.lbCurrentDate->setText(QDateTime::currentDateTime().toString("dd/MM/yyyy, hh:mm"));
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onIdleTimeout() {
    if (m_InputBox) {
        m_InputBox->deleteLater();
        m_InputBox = nullptr;
    }

    GUI::MessageBox::hide();

    m_Backend->toLog("Timeout AutoEncashment in service_menu scenario.");

    onExit();
}

//---------------------------------------------------------------------------
void AutoEncashmentWindow::onPanelChanged(int aIndex) {
    if (aIndex != 1) {
        return;
    }

    m_HistoryWindow->updateHistory();
}

//---------------------------------------------------------------------------