/* @file Базовый виджет для инкасации */

#include "EncashmentWindow.h"

#include <QtCore/QDebug>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>
#include <SDK/PaymentProcessor/Core/ServiceParameters.h>
#include <SDK/PaymentProcessor/Settings/UserSettings.h>

#include "Backend/HardwareManager.h"
#include "Backend/PaymentManager.h"
#include "Backend/ServiceMenuBackend.h"
#include "InputBox.h"
#include "MessageBox/MessageBox.h"
#include "ServiceTags.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
template <class T> void safeDelete(T *&aPointer) {
    if (aPointer) {
        aPointer->deleteLater();
        aPointer = nullptr;
    }
}

//---------------------------------------------------------------------------
EncashmentWindow::EncashmentWindow(ServiceMenuBackend *aBackend, QWidget *aParent)
    : QWidget(aParent), ServiceWindowBase(aBackend), m_EncashmentWithZReport(false),
      m_InputBox(nullptr), m_HistoryWindow(nullptr), m_LastPrintJob(0) {}

//---------------------------------------------------------------------------
EncashmentWindow::~EncashmentWindow() = default;

bool EncashmentWindow::activate() {
    connect(m_Backend->getPaymentManager(),
            SIGNAL(receiptPrinted(qint64, bool)),
            this,
            SLOT(onReceiptPrinted(qint64, bool)));
    return true;
}

//---------------------------------------------------------------------------
bool EncashmentWindow::deactivate() {
    safeDelete(m_InputBox);

    disconnect(m_Backend->getPaymentManager(),
               SIGNAL(receiptPrinted(qint64, bool)),
               this,
               SLOT(onReceiptPrinted(qint64, bool)));

    return true;
}

//---------------------------------------------------------------------------
void EncashmentWindow::doEncashment() {
    auto *paymentManager = m_Backend->getPaymentManager();
    bool isPrinterOK = paymentManager->canPrint(PPSDK::CReceiptType::Encashment);
    QString text = isPrinterOK ? tr("#question_encash") : tr("#question_encash_without_receipt");

    safeDelete(m_InputBox);

    if (GUI::MessageBox::question(text) != 0) {
        // Если баланс не пустой и нужно ввести номер кассеты
        if (paymentManager->getBalanceInfo()[CServiceTags::CashAmount].toDouble() > 0.0 &&
            dynamic_cast<PPSDK::UserSettings *>(
                m_Backend->getCore()->getSettingsService()->getAdapter(
                    PPSDK::CAdapterNames::UserAdapter))
                ->useStackerID()) {
            InputBox::ValidatorFunction validator = [](const QString &aText) -> bool {
                return !aText.trimmed().isEmpty();
            };
            m_InputBox = new InputBox(this, validator);
            m_InputBox->setLabelText(tr("#enter_stacker_id"));

            connect(m_InputBox, SIGNAL(accepted()), this, SLOT(doEncashmentProcess()));

            m_InputBox->show();
        } else {
            doEncashmentProcess();
        }
    }
}

//---------------------------------------------------------------------------
bool EncashmentWindow::doEncashmentProcess() {
    bool result = false;
    auto *paymentManager = m_Backend->getPaymentManager();
    bool printerOK = paymentManager->canPrint(PPSDK::CReceiptType::Encashment);

    m_IdleTimer.stop();

    QVariantMap parameters;

    if (m_InputBox) {
        parameters[PPSDK::EncashmentParameter::StackerID] = m_InputBox->textValue().trimmed();
        safeDelete(m_InputBox);
    }

    switch (paymentManager->perform(parameters)) {
    case PPSDK::EncashmentResult::OK:
        result = true;

        GUI::MessageBox::info(tr("#encashment_complete"));

        if (!printerOK) {
            m_MessageError = tr("#encashment_print_failed");
        } else {
            m_MessageSuccess = tr("#encashment_complete_and_printed");
            GUI::MessageBox::wait(tr("#printing"));
        }

        // Даже если принтер недоступен сохраним электронную копию чека инкассации
        paymentManager->printEncashment();

        // Сбросим счетчики отбракованных купюр/монет
        m_Backend->getCore()
            ->getService("FundsService")
            ->resetParameters(QSet<QString>() << PPSDK::CServiceParameters::Funds::RejectCount);
        break;

    case PPSDK::EncashmentResult::TryLater:
        GUI::MessageBox::critical(tr("#encashment_error_try_later"));
        break;

    default:
        GUI::MessageBox::critical(tr("#encashment_error"));
        break;
    }

    updateUI();

    return result;
}

//---------------------------------------------------------------------------
void EncashmentWindow::onPrintZReport() {
    auto *zReportButton = dynamic_cast<QPushButton *>(sender());
    GUI::MessageBox::hide();

    m_MessageError = tr("#zreport_failed");
    if (m_Backend->getPaymentManager()->canPrint(PPSDK::CReceiptType::ZReport)) {
        m_MessageSuccess = tr("#zreport_printed");
        bool fullZReport = false;
        bool canPrintFullZReport =
            m_Backend->getHardwareManager()->isFiscalPrinterPresent(false, true);

        QString msg =
            canPrintFullZReport ? tr("#print_full_zreport") : tr("#full_zreport_print_failed");

        if (canPrintFullZReport) {
            fullZReport = (GUI::MessageBox::question(msg) != 0);
        } else {
            GUI::MessageBox::warning(msg);
        }

        m_IdleTimer.stop();

        GUI::MessageBox::wait(tr("#printing"));

        // if (zReportButton)
        {
            int jobIndex = m_Backend->getPaymentManager()->printZReport(fullZReport);
            if (jobIndex == -1) {
                m_Backend->toLog(LogLevel::Debug, QString("JOB id=%1 CREATE FAIL.").arg(jobIndex));
                GUI::MessageBox::warning(tr("#full_zreport_print_failed"));
            } else {
                m_Backend->toLog(LogLevel::Debug, QString("JOB id=%1 CREATE.").arg(jobIndex));
            }
        }
    } else {
        // TODO Дополнять статусом принтера
        GUI::MessageBox::critical(m_MessageError);
    }
}

//------------------------------------------------------------------------
void EncashmentWindow::onReceiptPrinted(qint64 aJobIndex, bool aErrorHappened) {
    if ((m_LastPrintJob != 0) && m_LastPrintJob == aJobIndex) {
        m_Backend->toLog(LogLevel::Debug,
                         QString("JOB id=%1 ALREADY COMPLETE. SKIP SLOT.").arg(aJobIndex));
        GUI::MessageBox::hide();
        return;
    }

    m_LastPrintJob = aJobIndex;

    m_Backend->toLog(
        LogLevel::Debug,
        QString("JOB id=%1 COMPLETE. Error status: %2").arg(aJobIndex).arg(aErrorHappened));

    if (!m_MessageError.isEmpty() && aErrorHappened) {
        GUI::MessageBox::critical(m_MessageError);
    } else if (!m_MessageSuccess.isEmpty() && !aErrorHappened) {
        GUI::MessageBox::info(m_MessageSuccess);
    }

    m_MessageError.clear();
    m_MessageSuccess.clear();

    if (m_EncashmentWithZReport) {
        QTimer::singleShot(1000, this, SLOT(onPrintZReport()));

        m_EncashmentWithZReport = false;
    }

    m_IdleTimer.start();
    ;
}

//---------------------------------------------------------------------------
