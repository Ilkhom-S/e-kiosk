/* @file Реализация запроса статуса для пинового платежа. */

#include "PinPaymentStatusRequest.h"

PinPaymentStatusRequest::PinPaymentStatusRequest(Payment *aPayment)
    : PaymentStatusRequest(aPayment) {
    addParameter("PIN_DATA", 1);
}

//---------------------------------------------------------------------------
