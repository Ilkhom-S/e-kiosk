/* @file Окно инкассации. */

#include "EncashmentServiceWindow.h"

#include <QtCore/QSettings>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IFundsService.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Settings/UserSettings.h>

#include <SysUtils/ISysUtils.h>

#include "Backend/HardwareManager.h"
#include "Backend/HumoServiceBackend.h"
#include "Backend/PaymentManager.h"
#include "EncashmentHistoryWindow.h"
#include "InputBox.h"
#include "MessageBox/MessageBox.h"
#include "ServiceTags.h"

namespace {
const QString PayloadSettings = "payload.ini";
} // namespace

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------
EncashmentServiceWindow::EncashmentServiceWindow(HumoServiceBackend *aBackend, QWidget *aParent)
    : EncashmentWindow(aBackend, aParent), m_Backend(aBackend) {
    ui.setupUi(this);

    // TODO Заполнять значениями
    ui.twNotes->hide();

    ui.btnPayload->setVisible(false);

    /// Найдем файл настроек для диспенсеров
    // Используем reinterpret_cast через void* для корректной работы с multiple inheritance
    // См. docs/multiple-inheritance-rtti-casting.md
    void *terminalSettingsPtr =
        reinterpret_cast<void *>(m_Backend->getCore()->getSettingsService()->getAdapter(
            SDK::PaymentProcessor::CAdapterNames::TerminalAdapter));
    auto *s = reinterpret_cast<PPSDK::TerminalSettings *>(terminalSettingsPtr);

    m_PayloadSettingsPath =
        QString("%1/%2")
            .arg(s->getAppEnvironment().userDataPath)
            .arg(QString("%1_%2").arg(s->getKeys().value(0).ap).arg(PayloadSettings));

    if (QFile::exists(m_PayloadSettingsPath)) {
        QSettings settings(ISysUtils::rm_BOM(m_PayloadSettingsPath), QSettings::IniFormat);

        foreach (QString deviceGuid, settings.childGroups()) {
            m_PayloadSettings.insert(
                deviceGuid, settings.value(QString("%1/%2").arg(deviceGuid).arg("payload")));
        }

        /// Кнопка активна, если есть что загрузить в диспенсеры
        ui.btnPayload->setVisible(!m_PayloadSettings.isEmpty());
        connect(ui.btnPayload, SIGNAL(clicked()), this, SLOT(doPayload()));
    }

    m_HistoryWindow = new EncashmentHistoryWindow(aBackend, this);
    ui.gridLayoutEncashment->addWidget(m_HistoryWindow, 2, 0);
}

//------------------------------------------------------------------------
bool EncashmentServiceWindow::initialize() {
    updateUI();

    return true;
}

//------------------------------------------------------------------------
bool EncashmentServiceWindow::shutdown() {
    return true;
}

//----------------------------------------------------------------------------
void EncashmentServiceWindow::updateUI() {
    // Указываем сервису платежей, есть ли у нас аппаратный ФР
    m_Backend->getPaymentManager()->useHardwareFiscalPrinter(
        m_Backend->getHardwareManager()->isFiscalPrinterPresent(false));

    // Настраиваем интерфейс в зависимости от типа принтера
    bool isFiscalPrinter = m_Backend->getHardwareManager()->isFiscalPrinterPresent(true);

    ui.btnPrintZReport->setEnabled(
        isFiscalPrinter && m_Backend->getPaymentManager()->canPrint(PPSDK::CReceiptType::ZReport));
    ui.btnPrintBalance->setEnabled(
        m_Backend->getPaymentManager()->canPrint(PPSDK::CReceiptType::Balance));

    updateInfo();
}

//------------------------------------------------------------------------
bool EncashmentServiceWindow::activate() {
    EncashmentWindow::activate();

    connect(
        m_Backend->getHardwareManager(),
        SIGNAL(deviceStatusChanged(
            const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)),
        this,
        SLOT(onDeviceStatusChanged(
            const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)));

    updateUI();

    m_HistoryWindow->updateHistory();

    connect(ui.btnEncash, SIGNAL(clicked()), this, SLOT(doEncashment()));
    connect(ui.btnPrintBalance, SIGNAL(clicked()), this, SLOT(onPrintBalance()));
    connect(ui.btnPrintZReport, SIGNAL(clicked()), this, SLOT(onPrintZReport()));

    return true;
}

//------------------------------------------------------------------------
bool EncashmentServiceWindow::deactivate() {
    EncashmentWindow::deactivate();

    disconnect(
        m_Backend->getHardwareManager(),
        SIGNAL(deviceStatusChanged(
            const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)),
        this,
        SLOT(onDeviceStatusChanged(
            const QString &, const QString &, const QString &, SDK::Driver::EWarningLevel::Enum)));

    disconnect(ui.btnEncash, SIGNAL(clicked()), this, SLOT(doEncashment()));
    disconnect(ui.btnPrintBalance, SIGNAL(clicked()), this, SLOT(onPrintBalance()));
    disconnect(ui.btnPrintZReport, SIGNAL(clicked()), this, SLOT(onPrintZReport()));

    return EncashmentWindow::deactivate();
}

//------------------------------------------------------------------------
void EncashmentServiceWindow::onDeviceStatusChanged(const QString &aConfigName,
                                                    const QString &aStatusString,
                                                    const QString &aStatusColor,
                                                    SDK::Driver::EWarningLevel::Enum aLevel) {
    Q_UNUSED(aConfigName);
    Q_UNUSED(aStatusString);
    Q_UNUSED(aStatusColor);
    Q_UNUSED(aLevel);

    updateUI();
}

//------------------------------------------------------------------------
void EncashmentServiceWindow::updateInfo() {
    QVariantMap cashInfo = m_Backend->getPaymentManager()->getBalanceInfo();

    int noteCount = cashInfo[CServiceTags::NoteCount].toInt();
    int coinCount = cashInfo[CServiceTags::CoinCount].toInt();

    ui.lbLastEncashmentDate->setText(
        cashInfo[CServiceTags::LastEncashmentDate].toDateTime().toString("yyyy.MM.dd hh:mm"));
    ui.lbCashAmount->setText(cashInfo[CServiceTags::CashAmount].toString());
    ui.lbNoteCount->setText(QString("%1").arg(noteCount));
    ui.lbCoinCount->setText(QString("%1").arg(coinCount));

    ui.btnEncash->setEnabled(true);

    m_HistoryWindow->updateHistory();
}

//------------------------------------------------------------------------
void EncashmentServiceWindow::onPrintBalance() {
    m_MessageError = tr("#balance_print_failed");

    if (m_Backend->getPaymentManager()->canPrint(PPSDK::CReceiptType::Balance)) {
        m_MessageSuccess = tr("#balance_printed");
        m_Backend->getPaymentManager()->printBalance();
    } else {
        // TODO Дополнять статусом принтера
        GUI::MessageBox::critical(m_MessageError);
    }
}

//---------------------------------------------------------------------------
void EncashmentServiceWindow::doPayload() {
    PPSDK::TCashUnitsState cashUnitState =
        m_Backend->getCore()->getFundsService()->getDispenser()->getCashUnitsState();

    bool unitUpdateOK = false;

    QStringList devices = cashUnitState.keys();
    foreach (QString device, devices) {
        PPSDK::TCashUnitList cashUnits = cashUnitState.value(device);

        if (cashUnits.isEmpty()) {
            continue;
        }

        // номер кассеты:номинал:количество|номер кассеты:номинал:количество
        // обнулить все кассеты payload=0
        // обнулить первую кассету payload=1:0
        for (QString &unitPayload :
             m_PayloadSettings.value(device.split("_").last()).toString().split("|")) {
            if (unitPayload.isEmpty()) {
                continue;
            }

            QStringList payload = unitPayload.split(":");

            if (payload.count() == 1 && payload.first().toInt() == 0) {
                cashUnits.fill(PPSDK::SCashUnit());
            } else if (payload.count() == 2 && payload.first().toInt() < cashUnits.count() &&
                       payload.last().toInt() == 0) {
                cashUnits[payload.first().toInt()] = PPSDK::SCashUnit();
            } else if (payload.count() == 3) {
                PPSDK::SCashUnit &unit = cashUnits[payload.takeFirst().toInt()];
                unit.nominal = payload.takeFirst().toInt();
                unit.count = payload.takeFirst().toInt();
            } else {
                GUI::MessageBox::critical(tr("#check_update_payload_settings"));
            }
        }

        unitUpdateOK = m_Backend->getCore()->getFundsService()->getDispenser()->setCashUnitsState(
            device, cashUnits);
    }

    // Все кассеты обновились успешно
    if (unitUpdateOK) {
        ui.btnPayload->setVisible(false);
        QFile::remove(m_PayloadSettingsPath);
    }
}

//----------------------------------------------------------------------------
