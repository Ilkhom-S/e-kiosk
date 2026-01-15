/* @file Прокси класс для работы с купюроприёмниками и другими средствами приёма денег. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <Common/QtHeadersEnd.h>

#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Payment/Amount.h>

namespace SDK {
    namespace PaymentProcessor {

        class ICore;
        class IFundsService;

        namespace Scripting {

            //------------------------------------------------------------------------------
            /// Прокси класс для работы с купюроприёмниками и другими средствами приёма денег.
            class FundsService : public QObject {
                Q_OBJECT

              public:
                /// Конструктор.
                FundsService(ICore *aCore);

              public slots:
                /// Возвращает список доступных методов оплаты.
                QStringList getPaymentMethods() const;

                /// Начать приём денег для указанного платежа. Установить лимит aLimit для приёма.
                bool enable(qint64 aPayment, const QString &aPaymentMethod, QVariant aLimit);

                /// Начать приём денег для указанного платежа.
                bool enable(qint64 aPayment);

                /// Завершить приём денег для указанного платежа.
                bool disable(qint64 aPayment);

                /// Проверка, возможна ли выдача наличных средств
                bool canDispense();

                /// Выдать mAvailableAmount сумму (асинхронная операция)
                void dispense();

              private slots:
                /// Обработчик сигнала о мошенничестве
                void onCheated(qint64 aPayment);

                /// Обработчик системных событий
                void onEvent(const SDK::PaymentProcessor::Event &aEvent);

              signals:
                /// Сигнал об ошибке.
                void error(qint64 aPayment, QString aError);
                /// Сигнал о предупреждении.
                void warning(qint64 aPayment, QString aWarning);

                /// Сигнал об активности.
                void activity();

                /// Купюроприемник выключен.
                void disabled(qint64 aPayment);

                // Диспенсер
                /// Выдана указанная сумма.
                void dispensed(double aAmount);

                /// Сигнал об активности. Пример: купюра втянута в ящик сброса.
                void activity2();

                /// Сигнал срабатывает при ошибке приёма средств. В aError находится нелокализованная ошибка.
                void error2(QString aError);

              private:
                /// Указатель на ядро.
                ICore *mCore;
                /// Указатель на сервис средств.
                IFundsService *mFundsService;

                /// Сумма, которую может выдать диспенсер.
                TPaymentAmount mAvailableAmount;
            };

            //------------------------------------------------------------------------------
        } // namespace Scripting
    } // namespace PaymentProcessor
} // namespace SDK
