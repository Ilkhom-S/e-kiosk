/* @file Реализация проверочного платёжного запроса к серверу для пинового платежа. */

#pragma once

#include "PaymentCheckRequest.h"

//---------------------------------------------------------------------------
class PinPaymentCheckRequest : public PaymentCheckRequest {
public:
    PinPaymentCheckRequest(Payment *aPayment, bool aFake);
};

//---------------------------------------------------------------------------
