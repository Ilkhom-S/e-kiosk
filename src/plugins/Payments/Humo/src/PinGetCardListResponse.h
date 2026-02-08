/* @file Ответ на запрос получения номиналов pin-карт. */

#pragma once

#include <QtCore/QList>

#include <SDK/PaymentProcessor/Humo/Response.h>

#include "PinCard.h"

using namespace SDK::PaymentProcessor::Humo;

//---------------------------------------------------------------------------
class PinGetCardListResponse : public Response {
public:
    PinGetCardListResponse(const Request &aRequest, const QString &aResponseString);

    /// Предикат истинен, если ответ сервера не содержит ошибок.
    virtual bool isOk();

    /// Возвращает список описаний пин-карт, полученных из ответа сервера.
    virtual const QList<SPinCard> &getCards() const;

private:
    QList<SPinCard> m_Cards;
};

//---------------------------------------------------------------------------
