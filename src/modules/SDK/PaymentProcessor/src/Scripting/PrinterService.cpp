/* @file Прокси класс для работы с принтерами. */

#include <QtConcurrent/QtConcurrentRun>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Scripting/PrinterService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

namespace CPrintingService {
const int SaveReceiptJobID = 1234567;
} // namespace CPrintingService

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
PrinterService::PrinterService(ICore *aCore)
    : m_Core(aCore), m_PrinterService(m_Core->getPrinterService()),
      m_PaymentService(m_Core->getPaymentService()) {
    connect(m_PrinterService, SIGNAL(receiptPrinted(int, bool)), SLOT(onReceiptPrinted(int, bool)));
    connect(m_PrinterService, SIGNAL(printerStatus(bool)), SIGNAL(printerChecked(bool)));
}

//------------------------------------------------------------------------------
bool PrinterService::checkPrinter(bool aRealCheck) {
    if (aRealCheck) {
        // эта проверка должна вызываться крайне редко, т.к. в случае не ответа принтера подвешивает
        // интерфейс
        if (m_CheckSynchronizer.futures().size() == 0 ||
            m_CheckSynchronizer.futures().last().isFinished())
            m_CheckSynchronizer.addFuture(QtConcurrent::run([this]() { privateCheckPrinter(); }));

        return true;
    } else {
        return m_PrinterService->canPrintReceipt("payment", false);
    }
}

//------------------------------------------------------------------------------
bool PrinterService::checkFiscalRegister() {
    return m_PrinterService->hasFiscalRegister();
}

//------------------------------------------------------------------------------
void PrinterService::privateCheckPrinter() {
    emit printerChecked(m_PrinterService->canPrintReceipt("payment", true));
}

//------------------------------------------------------------------------------
void PrinterService::printReceiptExt(const QString &aReceiptType,
                                     const QVariantMap &aParameters,
                                     const QString &aTemplate,
                                     DSDK::EPrintingModes::Enum aPrintingMode) {
    qint64 paymentID = m_PaymentService->getActivePayment();

    if (paymentID > 0 && !aParameters.contains(SDK::PaymentProcessor::CPayment::Parameters::ID)) {
        QVariantMap aNewParameters(aParameters);
        aNewParameters.insert(SDK::PaymentProcessor::CPayment::Parameters::ID, paymentID);

        // Отправляем на печать с добавлением ID платежа
        m_PrintedJobs.insert(m_PrinterService->printReceipt(aReceiptType,
                                                          aNewParameters,
                                                          QString(aTemplate).replace(".xml", ""),
                                                          aPrintingMode),
                            TJobInfo(paymentID, aReceiptType));
    } else {
        // Отправляем на печать
        m_PrintedJobs.insert(
            m_PrinterService->printReceipt(
                aReceiptType, aParameters, QString(aTemplate).replace(".xml", ""), aPrintingMode),
            TJobInfo(paymentID, aReceiptType));
    }
}

//------------------------------------------------------------------------------
void PrinterService::printReceipt(const QString &aReceiptType,
                                  const QVariantMap &aParameters,
                                  const QString &aTemplate,
                                  bool aContinuousMode) {
    DSDK::EPrintingModes::Enum mode =
        aContinuousMode ? DSDK::EPrintingModes::Continuous : DSDK::EPrintingModes::None;
    printReceiptExt(aReceiptType, aParameters, aTemplate, mode);
}

//------------------------------------------------------------------------------
void PrinterService::saveReceipt(const QVariantMap &aParameters, const QString &aTemplate) {
    m_PrinterService->saveReceipt(aParameters, QString(aTemplate).replace(".xml", ""));

    emit receiptSaved();
}

//------------------------------------------------------------------------------
QString PrinterService::loadReceipt(qint64 aPaymentId) {
    return m_PrinterService->loadReceipt(aPaymentId == -1 ? m_PaymentService->getActivePayment()
                                                         : aPaymentId);
}

//------------------------------------------------------------------------------
bool PrinterService::checkReceiptMail() {
    return !(static_cast<PPSDK::TerminalSettings *>(
                 m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter))
                 ->getReceiptMailURL()
                 .isEmpty());
}

//------------------------------------------------------------------------------
void PrinterService::onReceiptPrinted(int aJobIndex, bool aError) {
    if (m_PrintedJobs.contains(aJobIndex)) {
        emit receiptPrinted(aError);

        TJobInfo job = m_PrintedJobs.value(aJobIndex);

        // В случае успеха отмечаем платеж как напечатанный
        if (!aError && job.first != 0 && !job.second.isEmpty()) {
            m_PaymentService->updatePaymentField(
                job.first,
                IPayment::SParameter(
                    SDK::PaymentProcessor::CPayment::Parameters::ReceiptPrinted, true, true),
                true);
        }

        m_PrintedJobs.remove(aJobIndex);
    }
}

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK
