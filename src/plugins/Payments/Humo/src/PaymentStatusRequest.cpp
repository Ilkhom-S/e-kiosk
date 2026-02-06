/* @file Реализация запроса статуса для пинового платежа. */

#include "PaymentStatusRequest.h"

#include "Payment.h"

//---------------------------------------------------------------------------
PaymentStatusRequest::PaymentStatusRequest(Payment *aPayment)
    : PaymentRequest(aPayment, CPayment::Requests::Status) {
    clear();

    addParameter("SD", aPayment->getKeySettings().sd);
    addParameter("AP", aPayment->getKeySettings().ap);
    addParameter("OP", aPayment->getKeySettings().op);

    addParameter("SESSION", aPayment->getSession());

    addProviderParameters(CPayment::Requests::Status);
}

//---------------------------------------------------------------------------
