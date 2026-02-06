/* @file Реализация запроса статуса платежа. */

#pragma once

#include "PaymentRequest.h"

//---------------------------------------------------------------------------
class PaymentStatusRequest : public PaymentRequest {
public:
    PaymentStatusRequest(Payment *aPayment);
};

//---------------------------------------------------------------------------
