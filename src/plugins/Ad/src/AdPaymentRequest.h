/* @file Реализация платёжного запроса к серверу. */

#pragma once

#include <QtCore/QStringList>

#include <SDK/PaymentProcessor/Humo/Request.h>

using namespace SDK::PaymentProcessor::Humo;

class AdPayment;

//---------------------------------------------------------------------------
class AdPaymentRequest : public Request {
public:
    //---------------------------------------------------------------------------
    // Конструктор запроса оплаты
    AdPaymentRequest(AdPayment *aPayment, const QString &aName);

    //---------------------------------------------------------------------------
    // Добавляет в запрос дополнительные параметры из описания оператора для указанного шага
    void addProviderParameters(const QString &aStep);

    //---------------------------------------------------------------------------
    // Возвращает платёж, ассоциированный с запросом
    virtual AdPayment *getPayment() const;

    //---------------------------------------------------------------------------
    // Возвращает название запроса
    virtual const QString &getName() const;

#pragma region Request interface

    //---------------------------------------------------------------------------
    // Возвращает содержимое запроса в пригодном для логирования виде
    virtual QString toLogString() const;

#pragma endregion

protected:
    /// Платёж, ассоциированный с запросом.
    AdPayment *m_Payment;

    /// Название запроса.
    QString m_Name;

private:
    /// Список полей, которые нельзя логировать.
    QStringList m_CryptedFields;
};

//---------------------------------------------------------------------------
