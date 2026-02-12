/* @file Реализация платёжного запроса к серверу. */

#pragma once

#include <QtCore/QStringList>

#include <SDK/PaymentProcessor/Humo/Request.h>

using namespace SDK::PaymentProcessor::Humo;

class Payment;

//---------------------------------------------------------------------------
class PaymentRequest : public Request {
public:
    PaymentRequest(Payment *aPayment, QString aName);

    /// Добавляет в запрос дополнительные параметры из описания оператора для указанного шага.
    void addProviderParameters(const QString &aStep);

    /// Возвращает платёж, ассоциированный с запросом.
    virtual Payment *getPayment() const;

    /// Возвращает название запроса.
    virtual const QString &getName() const;

protected:
    /// Платёж, ассоциированный с запросом.
    Payment *m_Payment;

    /// Название запроса.
    QString m_Name;
};

//---------------------------------------------------------------------------
