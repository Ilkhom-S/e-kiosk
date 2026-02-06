/* @file Интерфейс обеспечивающий взаимодействие с системой приёма средств. */

#pragma once

// Stl
#include <QtCore/QObject>
#include <QtCore/QtGlobal>

#include <SDK/PaymentProcessor/Payment/Amount.h>

#include <tuple>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class ICashAcceptorManager : public QObject {
    Q_OBJECT

public:
    /// Возвращает список доступных методов оплаты.
    virtual QStringList getPaymentMethods() = 0;

    /// Начать приём денег для указанного платежа.
    virtual bool
    enable(qint64 aPayment, const QString &aPaymentMethod, TPaymentAmount aMaxAmount) = 0;

    /// Завершить приём денег для указанного платежа. Возвращает false, если платеж еще
    /// обрабатывается.
    virtual bool disable(qint64 aPayment) = 0;

signals:
    /// Сигнал срабатывает, когда в платёж добавляется новая купюра.
    void amountUpdated(qint64 aPayment, double aTotalAmount, double aAmount);

    /// Сигнал срабатывает при ошибке приёма средств. В aError находится нелокализованная ошибка.
    void warning(qint64 aPayment, QString aError);

    /// Сигнал срабатывает при ошибке приёма средств. В aError находится нелокализованная ошибка.
    void error(qint64 aPayment, QString aError);

    /// Сигнал срабатывает при подозрении на манипуляции с устройством приема денег.
    void cheated(qint64 aPayment);

    /// Сигнал об активности сервиса. Пример: отбракована купюра.
    void activity();

    /// Сигнал о выключении купюроприемника на прием денег.
    void disabled(qint64 aPayment);

protected:
    virtual ~ICashAcceptorManager() {}
};

//------------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
