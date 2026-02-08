/* @file Дилерский платёж через процессинг Хумо. */

#pragma once

#include <QtCore/QSharedPointer>

#include <SDK/PaymentProcessor/Payment/IPaymentFactory.h>

#include "DealerLocalData.h"
#include "Payment.h"

//------------------------------------------------------------------------------
class DealerPayment : public Payment {
public:
    DealerPayment(PaymentFactory *aFactory);

protected:
    /// Запрос на проведение платежа.
    virtual bool check(bool aFakeCheck);

    /// Транзакция.
    virtual bool pay();

    /// Запрос статуса платежа.
    virtual bool status();

protected:
    /// Выставляет коды ошибок сервера в OK
    void setStateOk();

    bool haveLocalData();

    QString getAddinfo(QMap<QString, QString> &aValues);

    QString getAddFields();

protected:
    PaymentFactory *m_Factory;
    DealerLocalData m_LocalData;
};

//---------------------------------------------------------------------------
