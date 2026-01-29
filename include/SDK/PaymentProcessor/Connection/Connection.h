/* @file Структура описания соединения. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkProxy>
#include <Common/QtHeadersEnd.h>

// SDK
#include "SDK/PaymentProcessor/Connection/ConnectionTypes.h"

namespace SDK
{
    namespace PaymentProcessor
    {

        //----------------------------------------------------------------------------
        /// Константы соединения.
        namespace CConnection
        {
            // Интервал между пингами (в минутах) по умолчанию.
            const int DefaultCheckInterval = 15;
        } // namespace CConnection

        //----------------------------------------------------------------------------
        /// Шаблон соединения.
        struct SConnectionTemplate
        {
            QString name;
            QString initString;
            QString phone;
            QString login;
            QString password;
            QString balanceNumber;
            QString regExp;
        };

        //----------------------------------------------------------------------------
        /// Структура соединения.
        struct SConnection
        {
            /// Тип списка URL.
            /// Тип списка URL.
            typedef QList<QUrl> TUrlList;

            /// Конструктор.
            SConnection()
            {
                type = EConnectionTypes::Unknown;
                checkInterval = CConnection::DefaultCheckInterval;
                proxy = QNetworkProxy(QNetworkProxy::NoProxy);
            }

            /// Тип соединения.
            EConnectionTypes::Enum type;

            /// Название соединения.
            QString name;

            /// Прокси сервер (если есть).
            QNetworkProxy proxy;

            /// Интервал между пингами (в минутах).
            int checkInterval;

            /// Оператор сравнения.
            bool operator==(const SConnection &aCopy) const
            {
                return type == aCopy.type && name == aCopy.name && proxy == aCopy.proxy &&
                       checkInterval == aCopy.checkInterval;
            }
        };

        //----------------------------------------------------------------------------
    } // namespace PaymentProcessor
} // namespace SDK
