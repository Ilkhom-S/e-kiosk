/* @file Пиновый платёж через процессинг Хумо. */

#include "PinPayment.h"

#include "PinPaymentCheckRequest.h"
#include "PinPaymentStatusRequest.h"

//------------------------------------------------------------------------------
namespace CPin {
const char UIFieldName[] = "CARD";
} // namespace CPin

//------------------------------------------------------------------------------

PinPayment::PinPayment(PaymentFactory *aFactory) : Payment(aFactory) {}

//---------------------------------------------------------------------------
bool PinPayment::canProcessOffline() const {
    return false;
}

//---------------------------------------------------------------------------
Request *PinPayment::createRequest(const QString &aStep) {
    if (aStep == CPayment::Requests::FakeCheck) {
        return new PinPaymentCheckRequest(this, true);
    }
    if (aStep == CPayment::Requests::Check) {
        return new PinPaymentCheckRequest(this, false);
    }
    if (aStep == CPayment::Requests::Status) {
        return new PinPaymentStatusRequest(this);
    }

    return Payment::createRequest(aStep);
}

//---------------------------------------------------------------------------
bool PinPayment::getLimits(double &aMinAmount, double &aMaxAmount) {
    QList<SPinCard> cardList = getPaymentFactory()->getPinCardList(getProvider(false));

    QString cardId = getParameter(CPin::UIFieldName).value.toString();

    foreach (auto card, cardList) {
        if (card.id == cardId) {
            aMaxAmount = aMinAmount = card.amount;
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------
bool PinPayment::limitsDependOnParameter(const SParameter &aParameter) {
    return Payment::limitsDependOnParameter(aParameter) || (aParameter.name == CPin::UIFieldName);
}

//---------------------------------------------------------------------------
