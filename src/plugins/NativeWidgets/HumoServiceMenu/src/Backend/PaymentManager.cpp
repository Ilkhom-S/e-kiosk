/* @file Менеджер для работы с платежами */

// std

#include "PaymentManager.h"

#include <QtCore/QCache>
#include <QtCore/QDebug>

#include <SDK/Drivers/Components.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/ICryptService.h>
#include <SDK/PaymentProcessor/Core/IDeviceService.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Core/IPrinterService.h>
#include <SDK/PaymentProcessor/Core/IService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Core/ReceiptTypes.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Payment/Security.h>
#include <SDK/PaymentProcessor/Payment/Step.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <Crypt/ICryptEngine.h>
// #include <PaymentProcessor/PrintConstants.h>  // Removed - using local definitions
#include <algorithm>
#include <limits>

#include "../GUI/PaymentInfo.h"
#include "GUI/ServiceTags.h"

namespace PPSDK = SDK::PaymentProcessor;
namespace CPayment = PPSDK::CPayment::Parameters;

// Local PrintConstants definitions (workaround for linking issue)
namespace CPrintConstants {
const char OpBrand[] = "OPERATOR_BRAND";
const char ServiceType[] = "SERVICE_TYPE";
} // namespace CPrintConstants

//------------------------------------------------------------------------
namespace CPaymentManager {
const char UnprintedReest[] = "rup";
const char UnprintedPaymentList[] = "[UNPRINTED_PAYMENT_LIST]";
} // namespace CPaymentManager

//------------------------------------------------------------------------
PaymentManager::PaymentManager(PPSDK::ICore *aCore)
    : m_Core(aCore), m_UseFiscalPrinter(false), m_PaymentService(m_Core->getPaymentService()),
      m_PrinterService(m_Core->getPrinterService()), m_PaymentsRegistryPrintJob(0) {

    connect(m_PrinterService,
            SIGNAL(receiptPrinted(int, bool)),
            this,
            SLOT(onReceiptPrinted(int, bool)));

    connect(m_PaymentService,
            SIGNAL(stepCompleted(qint64, int, bool)),
            this,
            SIGNAL(paymentChanged(qint64)));

    // Используем reinterpret_cast через void* для корректной работы с multiple inheritance
    // См. docs/multiple-inheritance-rtti-casting.md
    void *dealerSettingsPtr = reinterpret_cast<void *>(
        m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::DealerAdapter));
    m_DealerSettings = reinterpret_cast<PPSDK::DealerSettings *>(dealerSettingsPtr);
}

//------------------------------------------------------------------------
PaymentManager::~PaymentManager() = default;

//------------------------------------------------------------------------
QVariantMap
PaymentManager::balanceParameters(const SDK::PaymentProcessor::SBalance &aBalance) const {
    QVariantMap cashInfo;
    if (aBalance.isValid) {
        cashInfo[CServiceTags::LastEncashmentDate] = aBalance.lastEncashmentDate;
        cashInfo[CServiceTags::CashAmount] = aBalance.amount;

        auto fields = aBalance.getFields();

        cashInfo[CServiceTags::NoteCount] = fields["BILL_COUNT"];
        cashInfo[CServiceTags::CoinCount] = fields["COIN_COUNT"];
    }

    return cashInfo;
}

//------------------------------------------------------------------------
QVariantMap PaymentManager::getBalanceInfo() const {
    return balanceParameters(m_PaymentService->getBalance());
}

//------------------------------------------------------------------------
QVariantMap PaymentManager::getEncashmentInfo(int aIndex) const {
    if (m_EncashmentList.size() > aIndex && aIndex >= 0) {
        auto encashment = m_EncashmentList[aIndex];
        auto info = balanceParameters(encashment.balance);

        info.insert(encashment.getFields());
        return info;
    }

    return {};
}

//------------------------------------------------------------------------
SDK::PaymentProcessor::EncashmentResult::Enum
PaymentManager::perform(const QVariantMap &aParameters) {
    m_Core->getEventService()->sendEvent(PPSDK::Event(PPSDK::EEventType::ProcessEncashment));

    if (m_PaymentService->getBalance().isEmpty()) {
        m_Encashment = m_PaymentService->getLastEncashment();

        return m_Encashment.isValid() ? PPSDK::EncashmentResult::OK
                                      : PPSDK::EncashmentResult::Error;
    }

    return m_PaymentService->perform_Encashment(aParameters, m_Encashment);
}

//------------------------------------------------------------------------
bool PaymentManager::canPrint(const QString &aReceiptType) const {
    return m_PrinterService->canPrintReceipt(aReceiptType, false);
}

//------------------------------------------------------------------------
QString PaymentManager::decryptParameter(const QString &aValue) {
    ICryptEngine *cryptEngine = m_Core->getCryptService()->getCryptEngine();

    QByteArray decryptedValue;

    QString error;

    if (cryptEngine->decryptLong(-1, aValue.toLatin1(), decryptedValue, error)) {
        return QString::fromUtf8(decryptedValue);
    }

    return "**DECRYPT ERROR**";
}

//------------------------------------------------------------------------
bool PaymentManager::printReceipt(qint64 aPaymentId, DSDK::EPrintingModes::Enum aPrintingMode) {
    QList<PPSDK::IPayment::SParameter> paymentParams =
        m_PaymentService->getPaymentFields(aPaymentId);
    qint64 providerId =
        PPSDK::IPayment::parameterByName(CPayment::Provider, paymentParams).value.toLongLong();
    // Используем reinterpret_cast через void* для корректной работы с multiple inheritance
    // См. docs/multiple-inheritance-rtti-casting.md
    void *dealerSettingsPtr = reinterpret_cast<void *>(
        m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::DealerAdapter));
    PPSDK::DealerSettings *dealerSettings =
        reinterpret_cast<PPSDK::DealerSettings *>(dealerSettingsPtr);
    PPSDK::SProvider provider = dealerSettings->getProvider(providerId);

    QString receiptTemplate =
        provider.receipts.contains("default")
            ? provider.receipts["default"].value<QString>().replace(".xml", "")
            : PPSDK::CReceiptType::Payment;

    QVariantMap receiptParameters;
    foreach (PPSDK::IPayment::SParameter parameter, paymentParams) {
        receiptParameters[parameter.name] =
            parameter.crypted ? decryptParameter(parameter.value.toString()) : parameter.value;
    }

    receiptParameters[CPrintConstants::OpBrand] = provider.name;

    foreach (QString parameter, provider.receiptParameters.keys()) {
        receiptParameters[parameter] = provider.receiptParameters[parameter];
    }

    int jonIndex = m_PrinterService->printReceipt(
        PPSDK::CReceiptType::Payment, receiptParameters, receiptTemplate, aPrintingMode, true);

    m_PaymentPrintJobs.insert(jonIndex, aPaymentId);

    return jonIndex != 0;
}

//------------------------------------------------------------------------
bool PaymentManager::printUnprintedReceiptsRegistry(const QSet<qint64> &aPayments) {
    struct PaymentAmounts {
        QVariantList sumAmounts;
        double sumAmountAll{0.0};
        double sumDealerFee{0.0};
        double sumProcessingFee{0.0};
        QStringList registry;
        QStringList paymentTitles;
        QVariantList paymentsVAT;
        QStringList paymentInn;

        PaymentAmounts() = default;
    };

    // группируем суммы по типу платежных средств
    QMap<int, PaymentAmounts> amounts;

    auto formatPaymentTitle = [](const PPSDK::SProvider &aProvider) -> QString {
        return QString("%1 (%2)")
            .arg(aProvider.receiptParameters[CPrintConstants::ServiceType].toString())
            .arg(aProvider.name);
    };

    auto payments = QList<qint64>(aPayments.begin(), aPayments.end());
    std::sort(payments.begin(), payments.end());

    foreach (qint64 id, payments) {
        QString session;
        QString amountAll;
        double amount = 0.0;
        double dealerFee = 0.0;
        double processingFee = 0.0;
        qint64 providerId = -1;
        qint64 gatewayIn = 0;
        qint64 gatewayOut = 0;
        int vat = 0;
        int payTool = 0;

        for (auto &parameter : m_PaymentService->getPaymentFields(id)) {
            if (parameter.name == CPayment::Amount) {
                amount = parameter.value.toDouble();
            } else if (parameter.name == CPayment::DealerFee) {
                dealerFee = parameter.value.toDouble();
            } else if (parameter.name == CPayment::ProcessingFee) {
                processingFee = parameter.value.toDouble();
            } else if (parameter.name == CPayment::AmountAll) {
                amountAll = parameter.value.toString();
            } else if (parameter.name == CPayment::InitialSession) {
                session = parameter.value.toString();
            } else if (parameter.name == CPayment::Provider) {
                providerId = parameter.value.toLongLong();
            } else if (parameter.name == CPayment::MNPGatewayIn) {
                gatewayIn = parameter.value.toLongLong();
            } else if (parameter.name == CPayment::MNPGatewayOut) {
                gatewayOut = parameter.value.toLongLong();
            } else if (parameter.name == CPayment::Vat) {
                vat = parameter.value.toInt();
            } else if (parameter.name == CPayment::PayTool) {
                payTool = parameter.value.toInt();
            }
        }

        if (!qFuzzyIsNull(amountAll.toDouble())) {
            auto provider = m_DealerSettings->getMNPProvider(providerId, gatewayIn, gatewayOut);

            amounts[payTool].sumAmountAll += amountAll.toDouble();
            amounts[payTool].sumAmounts << amount;
            amounts[payTool].paymentTitles << formatPaymentTitle(provider);
            amounts[payTool].paymentsVAT << vat;
            amounts[payTool].paymentInn
                << provider.receiptParameters.value("OPERATOR_INN").toString();
            amounts[payTool].sumDealerFee += dealerFee;
            amounts[payTool].sumProcessingFee += processingFee;

            amounts[payTool].registry << QString("%1 %2 %3").arg(id).arg(session).arg(amountAll);
        }
    }

    bool ok = false;

    foreach (int payTool, amounts.keys()) {
        if (!amounts[payTool].registry.isEmpty()) {
            QVariantMap receiptParameters;
            receiptParameters[CPaymentManager::UnprintedPaymentList] = amounts[payTool].registry;
            receiptParameters[CPayment::AmountAll] = amounts[payTool].sumAmountAll;
            receiptParameters[QString("[%1]").arg(QString(CPayment::Amount))] =
                amounts[payTool].sumAmounts;
            receiptParameters["[AMOUNT_TITLE]"] = amounts[payTool].paymentTitles;
            receiptParameters["[AMOUNT_VAT]"] = amounts[payTool].paymentsVAT;
            receiptParameters["[OPERATOR_INN]"] = amounts[payTool].paymentInn;
            receiptParameters[CPayment::PayTool] = payTool;
            receiptParameters[CPayment::DealerFee] = amounts[payTool].sumDealerFee;
            receiptParameters[CPayment::Fee] =
                amounts[payTool].sumDealerFee + amounts[payTool].sumProcessingFee;
            receiptParameters[CPayment::ProcessingFee] = amounts[payTool].sumProcessingFee;

            m_PaymentsRegistryPrintJob =
                m_PrinterService->printReceipt(PPSDK::CReceiptType::Payment,
                                               receiptParameters,
                                               CPaymentManager::UnprintedReest,
                                               DSDK::EPrintingModes::Continuous,
                                               true);

            ok = ok || (m_PaymentsRegistryPrintJob != 0);
        }
    }

    return ok;
}

//------------------------------------------------------------------------
void PaymentManager::onReceiptPrinted(int aJobIndex, bool aErrorHappened) {
    if (m_PaymentsRegistryPrintJob == aJobIndex) {
        if (!aErrorHappened) {
            // Обновить платежи - чек напечатан
            foreach (auto paymentId, m_Encashment.balance.notPrintedPayments) {
                m_PaymentService->updatePaymentField(
                    paymentId,
                    PPSDK::IPayment::SParameter(CPayment::ReceiptPrinted, true, true),
                    true);
            }

            m_Encashment.balance.notPrintedPayments.clear();
        }

        m_PaymentsRegistryPrintJob = 0;

        // Если была задача снять Z-отчёт и мы напечатали суммарный список платежей - выполняем сам
        // отчёт.
        if (!m_NeedPrintZReport.isEmpty() && aErrorHappened) {
            m_PrinterService->printReport(m_NeedPrintZReport, QVariantMap());

            m_NeedPrintZReport.clear();
        }
    } else if (m_PaymentPrintJobs.contains(aJobIndex)) {
        qint64 paymentId = m_PaymentPrintJobs.value(aJobIndex);

        if (!aErrorHappened) {
            // Обновить платёж - чек напечатан
            m_PaymentService->updatePaymentField(
                paymentId, PPSDK::IPayment::SParameter(CPayment::ReceiptPrinted, true, true), true);

            m_Encashment.balance.notPrintedPayments.remove(paymentId);

            emit paymentChanged(paymentId);
        }

        m_PaymentPrintJobs.remove(aJobIndex);
        emit receiptPrinted(paymentId, aErrorHappened);
    } else {
        emit receiptPrinted(aJobIndex, aErrorHappened);
    }
}

//------------------------------------------------------------------------
bool PaymentManager::printEncashment(int aIndex /*= -1*/) {
    PPSDK::SEncashment emptyEncashment;
    PPSDK::SEncashment &encashment = m_Encashment;

    if (aIndex >= 0) {
        encashment =
            (m_EncashmentList.size() > aIndex) ? m_EncashmentList[aIndex] : emptyEncashment;
    }

    if (!encashment.isValid()) {
        return false;
    }

    if (m_UseFiscalPrinter && aIndex < 0) {
        // Используем reinterpret_cast через void* для корректной работы с multiple inheritance
        // См. docs/multiple-inheritance-rtti-casting.md
        void *terminalSettingsPtr = reinterpret_cast<void *>(
            m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));
        PPSDK::TerminalSettings *terminalSettings =
            reinterpret_cast<PPSDK::TerminalSettings *>(terminalSettingsPtr);

        if (terminalSettings->getCommonSettings().printFailedReceipts) {
            // Перед отчётом печатаем все не напечатанные чеки
            printUnprintedReceiptsRegistry(encashment.balance.notPrintedPayments);
        } else {
            encashment.balance.notPrintedPayments.clear();
        }
    }

    auto fields = encashment.getFields();
    bool result = false;

    if (aIndex >= 0) {
        // печатаем копию инкассации
        fields.insert("NO_FISCAL", QVariant());
    }

    result = (m_PrinterService->printReport(PPSDK::CReceiptType::Encashment, fields) != 0);

    // Если есть устройство диспенсер
    if (!m_Core->getDeviceService()
             ->getConfigurations()
             .filter(QRegularExpression(DSDK::CComponents::Dispenser))
             .isEmpty()) {
        m_PrinterService->printReceipt(PPSDK::CReceiptType::DispenserEncashment,
                                       fields,
                                       PPSDK::CReceiptType::DispenserEncashment,
                                       DSDK::EPrintingModes::None,
                                       true);
    }

    return result;
}

//------------------------------------------------------------------------
bool PaymentManager::printTestPage() {
    return (m_PrinterService->printReceipt(PPSDK::CReceiptType::Test,
                                           QVariantMap(),
                                           PPSDK::CReceiptType::Test,
                                           DSDK::EPrintingModes::None,
                                           true) != 0);
}

//------------------------------------------------------------------------
bool PaymentManager::printBalance() const {
    auto fields = m_PaymentService->getBalance().getFields();
    bool result = (m_PrinterService->printReport(PPSDK::CReceiptType::Balance, fields) != 0);

    // Если есть устройство диспенсер
    if (!m_Core->getDeviceService()
             ->getConfigurations()
             .filter(QRegularExpression(DSDK::CComponents::Dispenser))
             .isEmpty()) {
        m_PrinterService->printReceipt(PPSDK::CReceiptType::DispenserBalance,
                                       fields,
                                       PPSDK::CReceiptType::DispenserBalance,
                                       DSDK::EPrintingModes::None,
                                       true);
    }

    return result;
}

//------------------------------------------------------------------------
int PaymentManager::printZReport(bool aFullZReport) {
    if (m_Encashment.balance.notPrintedPayments.isEmpty() || !m_UseFiscalPrinter) {
        return (m_PrinterService->printReport(aFullZReport ? PPSDK::CReceiptType::ZReportFull
                                                           : PPSDK::CReceiptType::ZReport,
                                              QVariantMap()) != 0)
                   ? 1
                   : 0;
    }
    int result = 0;
    if (m_PaymentsRegistryPrintJob == 0) {
        // Перед отчётом печатаем все не напечатанные чеки
        if (!printUnprintedReceiptsRegistry(m_Encashment.balance.notPrintedPayments)) {
            result = -1;
        }
    }

    // Оставляем пометку что мы хотели напечатать Z-отчет, но у нас есть не напечатанные чеки
    m_NeedPrintZReport =
        aFullZReport ? PPSDK::CReceiptType::ZReportFull : PPSDK::CReceiptType::ZReport;

    return result;
}

//------------------------------------------------------------------------
bool PaymentManager::getPaymentsInfo(QVariantMap & /*aPaymentsInfo*/) const {
    QVariantMap result;

    foreach (PPSDK::IService *service, m_Core->getServices()) {
        const QVariantMap &params = service->getParameters();
        for (auto it = params.begin(); it != params.end(); ++it) {
            result.insert(it.key(), it.value());
        }
    }

    // TODO Заполнить поля
    /*aPaymentsInfo[]
    lbRejectedBills->setText(QString::number(result[SDK::PaymentProcessor::CServiceParameters::Funds::RejectCount].toInt()));
    lbUnprocessedPayments->setText(QString::number(result[PPSDK::CServiceParameters::Payment::UnprocessedPaymentCount].toInt()));
    lbPrintedReceipts->setText(QString::number(result[SDK::PaymentProcessor::CServiceParameters::Printing::ReceiptCount].toInt()));
    lbRestartPerDay->setText(QString::number(result[SDK::PaymentProcessor::CServiceParameters::Terminal::RestartCount].toInt()));
    lbPaymentsPerDay->setText(QString::number(result[SDK::PaymentProcessor::CServiceParameters::Payment::PaymentsPerDay].toInt()));*/

    return true;
}

//------------------------------------------------------------------------
PaymentInfo PaymentManager::loadPayment(const QList<PPSDK::IPayment::SParameter> &aPaymentParams) {
    PaymentInfo paymentInfo;

    int status = PPSDK::IPayment::parameterByName(CPayment::Status, aPaymentParams).value.toInt();
    paymentInfo.setStatus(static_cast<PPSDK::EPaymentStatus::Enum>(status));

    paymentInfo.setPrinted(
        PPSDK::IPayment::parameterByName(CPayment::ReceiptPrinted, aPaymentParams).value.toBool());

    paymentInfo.setId(
        PPSDK::IPayment::parameterByName(CPayment::ID, aPaymentParams).value.toLongLong());
    paymentInfo.setAmount(
        PPSDK::IPayment::parameterByName(CPayment::Amount, aPaymentParams).value.toFloat());
    paymentInfo.setAmountAll(
        PPSDK::IPayment::parameterByName(CPayment::AmountAll, aPaymentParams).value.toFloat());
    paymentInfo.setCreationDate(
        PPSDK::IPayment::parameterByName(CPayment::CreationDate, aPaymentParams)
            .value.toDateTime());
    paymentInfo.setLastUpdate(
        PPSDK::IPayment::parameterByName(CPayment::LastUpdateDate, aPaymentParams)
            .value.toDateTime());

    paymentInfo.setSession(
        PPSDK::IPayment::parameterByName(CPayment::Session, aPaymentParams).value.toString());
    paymentInfo.setInitialSession(
        PPSDK::IPayment::parameterByName(CPayment::InitialSession, aPaymentParams)
            .value.toString());
    paymentInfo.setTransId(
        PPSDK::IPayment::parameterByName(CPayment::TransactionId, aPaymentParams).value.toString());

    qint64 providerId =
        PPSDK::IPayment::parameterByName(CPayment::Provider, aPaymentParams).value.toLongLong();

    PPSDK::SProvider provider = m_DealerSettings->getProvider(providerId);
    if (provider.name.isEmpty()) {
        paymentInfo.setProvider(paymentInfo.getStatus() == PPSDK::EPaymentStatus::LostChange
                                    ? tr("#lost_change")
                                    : QString::number(providerId));
    } else {
        paymentInfo.setProvider(QString("%1 (%2)").arg(provider.name).arg(providerId));
    }

    PPSDK::SecurityFilter filter(provider, PPSDK::SProviderField::SecuritySubsystem::Display);

    QStringList providerFields;
    foreach (PPSDK::SProviderField pf, provider.fields) {
        auto p = PPSDK::IPayment::parameterByName(pf.id, aPaymentParams);
        providerFields << pf.title + ": " +
                              filter.apply(pf.id,
                                           p.crypted ? decryptParameter(p.value.toString())
                                                     : p.value.toString());
    }

    paymentInfo.setProviderFields(providerFields.join("\n"));

    return paymentInfo;
}

//------------------------------------------------------------------------
void PaymentManager::updatePaymentList() {
    m_PaymentList.clear();

    QList<qint64> payments = m_PaymentService->getPayments(QSet<PPSDK::EPaymentStatus::Enum>());

    auto parameters = m_PaymentService->getPaymentsFields(payments);

    for (auto it = parameters.constBegin(); it != parameters.constEnd(); ++it) {
        m_PaymentList << loadPayment(it.value());
    }
}

//------------------------------------------------------------------------
QList<PaymentInfo> PaymentManager::getPayments(bool aNeedUpdate) {
    if (aNeedUpdate) {
        updatePaymentList();
    }

    return m_PaymentList;
}

//------------------------------------------------------------------------
void PaymentManager::processPayment(qint64 id) {
    if (m_PaymentService->canProcessPaymentOffline(id)) {
        m_PaymentService->processPayment(id, false);
    }
}

//------------------------------------------------------------------------
PaymentInfo PaymentManager::getPayment(qint64 id) {
    return loadPayment(m_PaymentService->getPaymentFields(id));
}

//------------------------------------------------------------------------
void PaymentManager::useHardwareFiscalPrinter(bool aUseFiscalPrinter) {
    m_UseFiscalPrinter = aUseFiscalPrinter;
}

//------------------------------------------------------------------------
int PaymentManager::getEncashmentsHistoryCount() {
    m_EncashmentList = m_PaymentService->getEncashmentList(10);

    // Удаляем первую "техническую" инкассацию
    m_EncashmentList.erase(std::remove_if(m_EncashmentList.begin(),
                                          m_EncashmentList.end(),
                                          [](const PPSDK::SEncashment &aEncashment) -> bool {
                                              return aEncashment.id == 1;
                                          }),
                           m_EncashmentList.end());

    const qsizetype count = m_EncashmentList.size();
    const auto maxCount = static_cast<qsizetype>(std::numeric_limits<int>::max());
    return static_cast<int>(count > maxCount ? maxCount : count);
}

//------------------------------------------------------------------------
