/* @file Управление терминалом. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

namespace SDK
{
    namespace PaymentProcessor
    {

        //---------------------------------------------------------------------------
        /// Список ошибок терминала.
        namespace ETerminalError
        {
            enum Enum
            {
                KeyError = 16001,
                ConfigError,
                NetworkError,
                DatabaseError,
                AccountBalanceError,
                InterfaceLocked
            };
        } // namespace ETerminalError

        //------------------------------------------------------------------------------
        /// Интерфейс управления терминалом.
        class ITerminalService
        {
          protected:
            /// Деструктор.
            virtual ~ITerminalService()
            {
            }

          public:
            //---------------------------------------------------------------------------
            /// Возможно ли проводить платежи?
            virtual bool isLocked() const = 0;

            //---------------------------------------------------------------------------
            /// Заблокировать/разблокировать терминал
            virtual void setLock(bool aIsLocked) = 0;

            //---------------------------------------------------------------------------
            /// Активировать загрузку конфигураций при следующем запуске
            virtual void needUpdateConfigs() = 0;

            //---------------------------------------------------------------------------
            /// Снять/поставить признак ошибки терминала
            virtual void setTerminalError(ETerminalError::Enum aErrorType, bool aError) = 0;

            //---------------------------------------------------------------------------
            /// Проверка наличия ошибки терминала
            virtual bool isTerminalError(ETerminalError::Enum aErrorType) const = 0;

            //---------------------------------------------------------------------------
            // Отправить сообщение разработчикам
            virtual void sendFeedback(const QString &aSenderSubsystem, const QString &aMessage) = 0;
        };

        //------------------------------------------------------------------------------
    } // namespace PaymentProcessor
} // namespace SDK
