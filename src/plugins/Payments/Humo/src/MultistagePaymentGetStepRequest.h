/* @file Запрос получения полей для следующего шага multistage шлюза. */

#pragma once

// Project
#include "Payment.h"
#include "PaymentRequest.h"

namespace CMultistage {
    namespace Protocol {
        const QString Step = "STEP";
        const QString Fields = "FIELDS";

        const QString FinalStepValue = "FINAL_STEP";
    } // namespace Protocol
} // namespace CMultistage

using namespace SDK::PaymentProcessor::Humo;

//---------------------------------------------------------------------------
class MultistagePaymentGetStepRequest : public PaymentRequest {
  public:
    MultistagePaymentGetStepRequest(Payment *aPayment);
};

//---------------------------------------------------------------------------
