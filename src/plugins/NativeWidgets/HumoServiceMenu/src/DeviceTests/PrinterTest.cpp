/* @file Класс для тестирования принтеров. */

#include "PrinterTest.h"

#include <SDK/Drivers/IFiscalPrinter.h>
#include <SDK/Drivers/IPrinter.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>

namespace CPrinterTest {
const QString PrintTestReceipt = QT_TRANSLATE_NOOP("PrinterTest", "#print_test_receipt");
} // namespace CPrinterTest

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
PrinterTest::PrinterTest(SDK::Driver::IDevice *aDevice, SDK::PaymentProcessor::ICore *aCore)
    : m_Printer(dynamic_cast<SDK::Driver::IPrinter *>(aDevice)), m_Core(aCore) {}

//------------------------------------------------------------------------------
QList<QPair<QString, QString>> PrinterTest::getTestNames() const {
    return QList<QPair<QString, QString>>() << qMakePair(CPrinterTest::PrintTestReceipt, QString());
}

//------------------------------------------------------------------------------
bool PrinterTest::run(const QString &aName) {
    if (aName == CPrinterTest::PrintTestReceipt) {
        m_TestResult = QtConcurrent::run([this]() {
            return m_Core->getPrinterService()->printReceiptDirected(
                m_Printer.data(), QString(PPSDK::CReceiptType::Test), QVariantMap());
        });
    }

    return true;
}

//------------------------------------------------------------------------------
void PrinterTest::stop() {
    m_TestResult.waitForFinished();
}

//------------------------------------------------------------------------------
bool PrinterTest::isReady() {
    if (m_Printer) {
        auto *fr = dynamic_cast<SDK::Driver::IFiscalPrinter *>(m_Printer.data());

        return fr ? fr->isFiscalReady(false, SDK::Driver::EFiscalPrinterCommand::Print)
                  : m_Printer->isDeviceReady(false);
    }

    return false;
}

//------------------------------------------------------------------------------
bool PrinterTest::hasResult() {
    return false;
}

//------------------------------------------------------------------------------
void PrinterTest::onPrinted(bool aError) {
    emit result(CPrinterTest::PrintTestReceipt, aError ? tr("#failed") : tr("#ok"));
}

//------------------------------------------------------------------------------