/* @file Ответ сервера на получения полей для следующего шага multistage шлюза. */

#include "MultistagePaymentGetStepResponse.h"

#include <QtXml/QDomDocument>

#include "MultistagePaymentGetStepRequest.h"
#include "PaymentRequest.h"

using namespace SDK::PaymentProcessor::Humo;

//---------------------------------------------------------------------------
MultistagePaymentGetStepResponse::MultistagePaymentGetStepResponse(const Request &aRequest,
                                                                   const QString &aResponseString)
    : PaymentResponse(aRequest, aResponseString), m_IsOk(false) {
    

    const auto *paymentRequest = dynamic_cast<const PaymentRequest *>(&aRequest);
    if (!paymentRequest) {
        return;
    }

    if (getError() != EServerError::Ok) {
        return;
    }

    QVariant step = getParameter(CMultistage::Protocol::Step);
    if (step.isNull()) {
        return;
    }

    m_Step = step.toString();

    QVariant fields = getParameter(CMultistage::Protocol::Fields);
    if (fields.isNull() && m_Step != CMultistage::Protocol::FinalStepValue) {
        return;
    }

    if (fields.isNull()) {
        m_Fields = "";
        m_IsOk = true;
    } else {
        m_Fields =
            QString::fromLocal8Bit(QByteArray::fromPercentEncoding(fields.toString().toLatin1()))
                .trimmed();

        QDomDocument doc("mydocument");
        auto result = doc.setContent(m_Fields);
        m_IsOk = m_Fields.isEmpty() ? true : static_cast<bool>(result);
    }
}

//---------------------------------------------------------------------------
bool MultistagePaymentGetStepResponse::isOk() {
    return (getError() == EServerError::Ok) && m_IsOk;
}

//---------------------------------------------------------------------------
QString MultistagePaymentGetStepResponse::getMultistageStep() const {
    return m_Step;
}

//---------------------------------------------------------------------------
QString MultistagePaymentGetStepResponse::getStepFields() const {
    return m_Fields;
}

//---------------------------------------------------------------------------
