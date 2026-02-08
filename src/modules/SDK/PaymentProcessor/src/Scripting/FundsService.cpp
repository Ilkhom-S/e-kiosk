/* @file Прокси класс для работы с купюроприёмниками и другими средствами приёма денег. */

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/IFundsService.h>
#include <SDK/PaymentProcessor/Core/IPaymentService.h>
#include <SDK/PaymentProcessor/Payment/Parameters.h>
#include <SDK/PaymentProcessor/Scripting/FundsService.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
FundsService::FundsService(ICore *aCore)
    : m_Core(aCore), m_FundsService(m_Core->getFundsService()), m_AvailableAmount(0.0) {
    connect(m_FundsService->getAcceptor(),
            SIGNAL(error(qint64, QString)),
            SIGNAL(error(qint64, QString)));
    connect(m_FundsService->getAcceptor(),
            SIGNAL(warning(qint64, QString)),
            SIGNAL(warning(qint64, QString)));
    connect(m_FundsService->getAcceptor(), SIGNAL(cheated(qint64)), SLOT(onCheated(qint64)));
    connect(m_FundsService->getAcceptor(), SIGNAL(activity()), SIGNAL(activity()));
    connect(m_FundsService->getAcceptor(), SIGNAL(disabled(qint64)), SIGNAL(disabled(qint64)));

    connect(m_FundsService->getDispenser(), SIGNAL(error(QString)), SIGNAL(error2(QString)));
    connect(m_FundsService->getDispenser(), SIGNAL(activity()), SIGNAL(activity2()));
    connect(m_FundsService->getDispenser(), SIGNAL(dispensed(double)), SIGNAL(dispensed(double)));

    m_Core->getEventService()->subscribe(this, SLOT(onEvent(const SDK::PaymentProcessor::Event)));
}

//------------------------------------------------------------------------------
bool FundsService::enable(qint64 aPayment) {
    return enable(aPayment, "", 0.0);
}

//------------------------------------------------------------------------------
bool FundsService::enable(qint64 aPayment, const QString &aPaymentMethod, QVariant aLimit) {
    return m_FundsService->getAcceptor()->enable(aPayment, aPaymentMethod, aLimit.toDouble());
}

//------------------------------------------------------------------------------
bool FundsService::disable(qint64 aPayment) {
    return m_FundsService->getAcceptor()->disable(aPayment);
}

//------------------------------------------------------------------------------
QStringList FundsService::getPaymentMethods() const {
    return m_FundsService->getAcceptor()->getPaymentMethods();
}

//------------------------------------------------------------------------------
bool FundsService::canDispense() {
    TPaymentAmount aRequiredAmount = m_Core->getPaymentService()->getChangeAmount();
    m_AvailableAmount = m_FundsService->getDispenser()->canDispense(aRequiredAmount);

    return !qFuzzyIsNull(m_AvailableAmount);
}

//------------------------------------------------------------------------------
void FundsService::dispense() {
    if (!qFuzzyIsNull(m_AvailableAmount)) {
        m_FundsService->getDispenser()->dispense(m_AvailableAmount);
        m_AvailableAmount = 0;
    }
}

//------------------------------------------------------------------------------
void FundsService::onCheated(qint64 aPayment) {
    if (aPayment > 0) {
        m_Core->getPaymentService()->updatePaymentField(
            aPayment,
            IPayment::SParameter(SDK::PaymentProcessor::CPayment::Parameters::Cheated,
                                 SDK::PaymentProcessor::EPaymentCheatedType::CashAcceptor,
                                 true,
                                 false,
                                 true));
    }
}

//------------------------------------------------------------------------------
void FundsService::onEvent(const SDK::PaymentProcessor::Event &aEvent) {
    if (aEvent.getType() == SDK::PaymentProcessor::EEventType::Critical) {
        auto paymentId = m_Core->getPaymentService()->getActivePayment();

        if (paymentId) {
            emit error(paymentId, aEvent.getData().toString());
        }
    }
}

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK
