/* @file Описание суммы платежа. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QMetaType>
#include <Common/QtHeadersEnd.h>

// Project
#include <Common/Currency.h>

namespace SDK
{
    namespace PaymentProcessor
    {

        //------------------------------------------------------------------------------
        /// Тип суммы платежа.
        namespace EAmountType
        {
            enum Enum
            {
                Bill = 0,
                Coin,
                EMoney,
                BankCard
            };
        } // namespace EAmountType

        //------------------------------------------------------------------------------
        /// Данные о зачисленной купюре.
        struct SNote
        {
            EAmountType::Enum type;    /// Тип валюты (купюры/монеты).
            Currency::Nominal nominal; /// Номинал.
            int currency;              /// ID валюты.
            QString serial;            /// Серийный номер принятой купюры.

            /// Конструктор.
            SNote(EAmountType::Enum aType = EAmountType::Bill, double aNominal = 0.0, int aCurrency = -1,
                  const QString &aSerial = "default")
                : type(aType), nominal(aNominal), serial(aSerial), currency(aCurrency)
            {
            }
        };

        //------------------------------------------------------------------------------
        /// Тип суммы платежа.
        typedef double TPaymentAmount;
        /// Карта сумм платежей.
        typedef QMap<QString, TPaymentAmount> TPaymentAmounts;

        //------------------------------------------------------------------------------
    } // namespace PaymentProcessor
} // namespace SDK

// Q_DECLARE_METATYPE(SDK::PaymentProcessor::SNote);
