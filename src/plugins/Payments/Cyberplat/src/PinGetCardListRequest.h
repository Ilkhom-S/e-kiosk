/* @file Запрос получения номиналов pin-карт. */

#pragma once

// SDK
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>
#include <SDK/PaymentProcessor/Humo/Request.h>

using namespace SDK::PaymentProcessor::Humo;

//---------------------------------------------------------------------------
class PinGetCardListRequest : public Request {
  public:
    PinGetCardListRequest(const SDK::PaymentProcessor::SKeySettings &aKeySettings);
};

//---------------------------------------------------------------------------
