/* @file Типы системных событий. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

namespace SDK {
    namespace PaymentProcessor {

        //---------------------------------------------------------------------------
        /// Типы рекламных событий.
        class AdvertisingType : public QObject {
            Q_OBJECT
            Q_ENUMS(Enum)

          public:
            enum Enum {
                MainScreenBanner, /// Баннер на первом экране.
                PaymentReceipt,   /// Текстовая реклама на чеке платежа.
            };
        };

        //---------------------------------------------------------------------------
    } // namespace PaymentProcessor
} // namespace SDK
