/* @file Базовый платёж через процессинг Хумо. */

#pragma once

#include <Payment/PaymentBase.h>

#include "PaymentFactory.h"

//------------------------------------------------------------------------------
class Payment : public PaymentBase {
    friend class PaymentFactory;

public:
    Payment(PaymentFactory *aFactory);

    /// Возвращает связанную фабрику платежей.
    PaymentFactory *getPaymentFactory() const;

#pragma region SDK::PaymentProcessor::IPayment interface

    /// Выполнение шага с идентификатором aStep.
    virtual bool perform_Step(int aStep);

    /// Обновление статуса платежа.
    virtual void process();

    /// Отметка платежа как удаленного. В случае успеха возвращает true.
    virtual bool remove();

#pragma endregion

protected:
    /// Создаёт класс запроса по идентификатору шага.
    virtual Request *createRequest(const QString &aStep);

    /// Создаёт класс ответа по классу запроса.
    virtual Response *createResponse(const Request &aRequest, const QString &aResponseString);

    /// Отправка запроса.
    virtual Response *sendRequest(const QUrl &aUrl, Request &aRequest);

    /// Запрос на проведение платежа.
    virtual bool check(bool aFakeCheck);

    /// Транзакция.
    virtual bool pay();

    /// Запрос статуса платежа.
    virtual bool status();

    /// Попытка проведения платежа.
    virtual void perform_Transaction();

    /// Проведение PAY части платежа
    void perform_TransactionPay();

    /// Пересчитываем кол-во попыток и время следующей
    void updateNumberOfTries();

    /// Критическая ошибка, проведение платежа прекращается.
    virtual bool isCriticalError(int aError) const;

    /// При ошибке проведения устанавливает таймауты для следующей попытки.
    virtual void setProcessError();

protected:
    RequestSender m_RequestSender;
};

//------------------------------------------------------------------------------
