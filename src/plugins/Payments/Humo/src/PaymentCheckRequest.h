/* @file Реализация запроса на проверку номера. */

#pragma once

#include "PaymentRequest.h"

//---------------------------------------------------------------------------
class PaymentCheckRequest : public PaymentRequest {
public:
    PaymentCheckRequest(Payment *aPayment, bool aFake);
};

//---------------------------------------------------------------------------
