/* @file Окно диагностики. */

#include "DiagnosticsServiceWindow.h"

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QSet>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>

#include "Backend/HardwareManager.h"
#include "Backend/HumoServiceBackend.h"
#include "Backend/NetworkManager.h"
#include "Backend/PaymentManager.h"
#include "DeviceStatusWindow.h"
#include "ServiceTags.h"

DiagnosticsServiceWindow::DiagnosticsServiceWindow(HumoServiceBackend *aBackend, QWidget *aParent)
    : QFrame(aParent), ServiceWindowBase(aBackend), m_SpacerItem(nullptr) {
    setupUi(this);

    connect(
        m_Backend->getHardwareManager(),
        SIGNAL(deviceStatusChanged(
            const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)),
        this,
        SLOT(onDeviceStatusChanged(
            const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)));

    connect(btnInfoPanel, SIGNAL(clicked()), this, SLOT(onClickedEncashmentInfo()));
    connect(btnTestServer, SIGNAL(clicked()), this, SLOT(onClickedTestServer()));
    connect(&m_TaskWatcher, SIGNAL(finished()), SLOT(onTestServerFinished()));
    connect(btnResetReject, SIGNAL(clicked()), this, SLOT(onClickedResetReject()));
    connect(btnResetReceipts, SIGNAL(clicked()), this, SLOT(onClickedResetReceipts()));

    // TODO Реализовать функционал
    lbTitleZReportCount->hide();
    lbZReportCount->hide();
    lbTitleSessionStatus->hide();
    lbSessionStatus->hide();

    onClickedTestServer();
}

//------------------------------------------------------------------------
bool DiagnosticsServiceWindow::activate() {
    onClickedEncashmentInfo();

    foreach (DeviceStatusWindow *widget, m_DeviceStatusWidget.values()) {
        widget->deleteLater();
    }

    m_DeviceStatusWidget.clear();

    vlTestWidgets->removeItem(m_SpacerItem);
    delete m_SpacerItem;
    m_SpacerItem = nullptr;

    // Получаем список устройств из конфигов.
    QStringList configNames = m_Backend->getHardwareManager()->getConfigurations();

    foreach (const QString config, configNames) {
        auto *dtw = new DeviceStatusWindow(m_Backend, config, this);
        vlTestWidgets->addWidget(dtw->getWidget());

        m_DeviceStatusWidget[config] = dtw;
    }

    m_Backend->getHardwareManager()->updateStatuses();

    m_SpacerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
    vlTestWidgets->layout()->addItem(m_SpacerItem);

    updateInfoPanel();

    return true;
}

//------------------------------------------------------------------------
bool DiagnosticsServiceWindow::deactivate() {
    return true;
}

//------------------------------------------------------------------------
bool DiagnosticsServiceWindow::initialize() {
    return true;
}

//------------------------------------------------------------------------
bool DiagnosticsServiceWindow::shutdown() {
    m_TaskWatcher.waitForFinished();

    return true;
}

//------------------------------------------------------------------------
void DiagnosticsServiceWindow::onDeviceStatusChanged(const QString &aConfigurationName,
                                                     const QString &aNewStatus,
                                                     const QString &aStatusColor,
                                                     SDK::Driver::EWarningLevel::Enum aLevel) {
    DeviceStatusWindow *widget = m_DeviceStatusWidget[aConfigurationName];
    if (widget) {
        widget->updateDeviceStatus(aNewStatus, aStatusColor, aLevel);
    }
}

//------------------------------------------------------------------------
void DiagnosticsServiceWindow::onClickedEncashmentInfo() {
    bool state = btnInfoPanel->isChecked();

    state ? btnInfoPanel->setText(tr("#title_turn_off"))
          : btnInfoPanel->setText(tr("#title_turn_on"));
    scrollArea_2->setVisible(state);
}

//------------------------------------------------------------------------
void DiagnosticsServiceWindow::updateInfoPanel() {
    QVariantMap result;

    foreach (SDK::PaymentProcessor::IService *service, m_Backend->getCore()->getServices()) {
        result.insert(service->getParameters());
    }

    lbSimBalance->setText(
        result[SDK::PaymentProcessor::CServiceParameters::Networking::SimBalance].toString());
    lbRejectedBills->setText(QString::number(
        result[SDK::PaymentProcessor::CServiceParameters::Funds::RejectCount].toInt()));
    lbUnprocessedPayments->setText(QString::number(
        result[SDK::PaymentProcessor::CServiceParameters::Payment::UnprocessedPaymentCount]
            .toInt()));
    lbPrintedReceipts->setText(QString::number(
        result[SDK::PaymentProcessor::CServiceParameters::Printing::ReceiptCount].toInt()));
    lbRestartPerDay->setText(QString::number(
        result[SDK::PaymentProcessor::CServiceParameters::Terminal::RestartCount].toInt()));
    lbPaymentsPerDay->setText(QString::number(
        result[SDK::PaymentProcessor::CServiceParameters::Payment::PaymentsPerDay].toInt()));

    QVariant param(result[SDK::PaymentProcessor::CServiceParameters::Printing::SessionStatus]);
    lbSessionStatus->setText(param.isNull() ? "-/-" : param.toString());
    lbSessionStatus->setEnabled(!param.isNull());
    lbTitleSessionStatus->setEnabled(!param.isNull());

    param = result[SDK::PaymentProcessor::CServiceParameters::Printing::SessionStatus];
    lbZReportCount->setText(param.isNull() ? "-/-" : QString::number(param.toInt()));
    lbZReportCount->setEnabled(!param.isNull());
    lbTitleZReportCount->setEnabled(!param.isNull());

    QVariantMap cashInfo = m_Backend->getPaymentManager()->getBalanceInfo();
    lbLastEncashmentDate->setText(cashInfo[CServiceTags::LastEncashmentDate].toString());

    bool isRoleTechician = m_Backend->getUserRole() == CServiceTags::UserRole::RoleTechnician;
    lbAmount->setText(isRoleTechician ? "-/-" : cashInfo[CServiceTags::CashAmount].toString());
    lbNotesCount->setText(
        isRoleTechician ? "-/-" : QString::number(cashInfo[CServiceTags::NoteCount].toInt()));
    lbCoinsCount->setText(
        isRoleTechician ? "-/-" : QString::number(cashInfo[CServiceTags::CoinCount].toInt()));
}

//---------------------------------------------------------------------------
void DiagnosticsServiceWindow::resetParameter(const QString &aParameterName) {
    QSet<QString> parameters;
    parameters << aParameterName;

    foreach (SDK::PaymentProcessor::IService *service, m_Backend->getCore()->getServices()) {
        service->resetParameters(parameters);
    }
}

//---------------------------------------------------------------------------
void DiagnosticsServiceWindow::onClickedTestServer() {
    btnTestServer->setEnabled(false);
    lbNetworkStatus->setText(tr("#connection_checking_status"));
    m_TaskWatcher.setFuture(QtConcurrent::run([this]() {
        QString errorMessage;
        return m_Backend->getNetworkManager()->testConnection(errorMessage);
    }));
}

//---------------------------------------------------------------------------
void DiagnosticsServiceWindow::onTestServerFinished() {
    btnTestServer->setEnabled(true);
    m_TaskWatcher.result() ? lbNetworkStatus->setText(tr("#connection_test_ok"))
                           : lbNetworkStatus->setText(tr("#connection_test_failed"));
}

//---------------------------------------------------------------------------
void DiagnosticsServiceWindow::onClickedResetReject() {
    resetParameter(SDK::PaymentProcessor::CServiceParameters::Funds::RejectCount);
    updateInfoPanel();
}

//---------------------------------------------------------------------------
void DiagnosticsServiceWindow::onClickedResetReceipts() {
    resetParameter(SDK::PaymentProcessor::CServiceParameters::Printing::ReceiptCount);
    updateInfoPanel();
}

//------------------------------------------------------------------------
