/* @file Реализация запроса на проведение платежа. */

#pragma once

#include "PaymentRequest.h"

//---------------------------------------------------------------------------
class PaymentPayRequest : public PaymentRequest {
public:
    PaymentPayRequest(Payment *aPayment);
};

//---------------------------------------------------------------------------
