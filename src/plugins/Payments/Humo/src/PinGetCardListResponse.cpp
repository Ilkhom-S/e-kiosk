/* @file Ответ на запрос получения номиналов pin-карт. */

#include "PinGetCardListResponse.h"

PinGetCardListResponse::PinGetCardListResponse(const Request &aRequest,
                                               const QString &aResponseString)
    : Response(aRequest, aResponseString) {
    if (getError() != EServerError::Ok) {
        return;
    }

    foreach (QString rawCard, getParameter("CARD_LIST").toString().split(":")) {
        QStringList cardParams = rawCard.split("=", Qt::KeepEmptyParts);
        if (cardParams.size() < 3) {
            continue;
        }

        SPinCard card;
        card.name = cardParams.takeFirst();
        card.id = cardParams.takeFirst();
        card.amount = cardParams.takeFirst().trimmed().toDouble();
        card.fields = cardParams;

        m_Cards << card;
    }
}

//---------------------------------------------------------------------------
bool PinGetCardListResponse::isOk() {
    return (getError() == EServerError::Ok);
}

//---------------------------------------------------------------------------
const QList<SPinCard> &PinGetCardListResponse::getCards() const {
    return m_Cards;
}

//---------------------------------------------------------------------------
