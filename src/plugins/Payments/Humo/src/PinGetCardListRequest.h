/* @file Запрос получения номиналов pin-карт. */

#pragma once

#include <SDK/PaymentProcessor/Humo/Request.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

using namespace SDK::PaymentProcessor::Humo;

//---------------------------------------------------------------------------
class PinGetCardListRequest : public Request {
public:
    PinGetCardListRequest(const SDK::PaymentProcessor::SKeySettings &aKeySettings);
};

//---------------------------------------------------------------------------
