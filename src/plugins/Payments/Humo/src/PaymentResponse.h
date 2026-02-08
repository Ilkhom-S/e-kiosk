/* @file Ответ сервера на платёжный запрос. */

#pragma once

#include <QtCore/QStringList>

#include <Common/ILogable.h>

#include <SDK/PaymentProcessor/Humo/Response.h>

using namespace SDK::PaymentProcessor::Humo;

//---------------------------------------------------------------------------
class PaymentResponse : public Response, private ILogable {
public:
    PaymentResponse(const Request &aRequest, const QString &aResponseString);

#pragma region Response interface

    /// Возвращает содержимое запроса в пригодном для логирования виде.
    virtual QString toLogString() const;

#pragma endregion

private:
    /// Список полей, которые нельзя логировать.
    QStringList m_CryptedFields;
};

//---------------------------------------------------------------------------
