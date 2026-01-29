/* @file Типы системных событий. */

#pragma once

#include <Common/QtHeadersBegin.h>
#include <QtCore/QObject>
#include <Common/QtHeadersEnd.h>

namespace SDK
{
    namespace PaymentProcessor
    {

        //---------------------------------------------------------------------------
        // Сделан в виде класса, чтобы получить метаданные.
        class EEventType
        {
            // Use Q_NAMESPACE to get metadata without the overhead of a full QObject class
            Q_GADGET
          public:
            enum Enum
            {
                Unknown = 0, /// Неизвестный тип события.

                TerminalLock,   /// Блокировка терминала.
                TerminalUnlock, /// Разблокировка терминала.

                InitApplication,      /// Инициализация приложения.
                ReinitializeServices, /// Произвести пере инициализацию всех сервисов.
                CloseApplication,     /// Корректная остановка программы (если допускается активной логикой).
                TerminateApplication, /// Принудительное завершение работы.

                Reboot,   /// Перезагрузка системы.
                Restart,  /// Перезапуск ТК.
                Shutdown, /// Выключить терминал.

                PaymentUpdated, /// Изменился статус платежа.

                ConnectionEstablished, /// Интернет соединение установлено.
                ConnectionLost,        /// Интернет соединение потеряно.

                DesktopActivity, /// Пользовательские клики на экране WatchService.

                StateChanged, /// Состояние модуля изменилось.

                StartScenario,   /// Запуск нового сценария.
                UpdateScenario,  /// Состояние выполняемого сценария изменилось.
                StopScenario,    /// Остановка сценария. Производит откат к ранее запущенному, если такой есть.
                TryStopScenario, /// Запрос на остановку сценария. Сценарий должен сам решить - будет ли он
                                 /// останавливаться.

                RestoreConfiguration, /// Скачать файлы с сервера.
                StopSoftware,         /// Остановка всех приложений.
                Autoencashment,       /// Автоинкассация.

                StartGraphics, /// Запустить графический движок
                PauseGraphics, /// Поставить на паузу
                StopGraphics,  /// Остановить

                OK,       /// Сообщение OK. Попадает в мониторинг в качестве статуса терминала.
                Warning,  /// Сообщение о некритичной ситуации. Попадает в мониторинг в качестве статуса терминала.
                Critical, /// Сообщение о критической ситуации. Попадает в мониторинг в качестве статуса терминала.

                ProcessEncashment, /// Команда провести инкассацию всех компонент
            };

            Q_ENUM(Enum)
        }; // namespace EEventType

        //---------------------------------------------------------------------------
    } // namespace PaymentProcessor
} // namespace SDK