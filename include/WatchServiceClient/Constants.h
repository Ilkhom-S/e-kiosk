/* @file Константы клиента сторожевого сервиса. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

#define WATCHSERVICE_TCP

//---------------------------------------------------------------------------
namespace CWatchService
{
    /// Название сервиса.
    const QString ServiceName = "Humo Watch Service";

    /// Имя сервиса.
    const QString Name = "watch_service";

#ifdef WATCHSERVICE_TCP
    /// Очередь сообщений.
    const QString MessageQueue = "12346";
#else
    const QString MessageQueue = "WatchServiceMessageQueue";
#endif // WATCHSERVICE_TCP

    /// Рекомендуемый временной интервал для пинга сторожевого сервиса.
    const int PingInterval = 5 * 1000; // msec

    /// Время, после которого в случае отсутствия пингов, сторожевой сервис закроет клиента.
    const int MaxPingInterval = 60 * 1000; // msec

    namespace Commands
    {
        /// Команда пинга.
        const QString Ping = "ping";
        /// Команда активности экрана.
        const QString ScreenActivity = "screen_activity";
        /// Команда закрытия.
        const QString Close = "close";
        /// Команда выхода.
        const QString Exit = "exit";
        /// Команда запуска модуля.
        const QString StartModule = "start_module";
        /// Команда закрытия модуля.
        const QString CloseModule = "close_module";
        /// Команда перезапуска.
        const QString Restart = "restart";
        /// Команда перезагрузки.
        const QString Reboot = "reboot";
        /// Команда выключения.
        const QString Shutdown = "shutdown";
        /// Команда показа заставки.
        const QString ShowSplashScreen = "show_splash_screen";
        /// Команда скрытия заставки.
        const QString HideSplashScreen = "hide_splash_screen";
        /// Команда установки состояния.
        const QString SetState = "set_state";
        /// Команда сброса состояния.
        const QString ResetState = "reset_state";
        /// Команда закрытия логов.
        const QString CloseLogs = "close_logs";
    } // namespace Commands

    namespace Notification
    {
        /// Уведомление о закрытии модуля.
        const QString ModuleClosed = "module_closed";
    } // namespace Notification

    namespace Modules
    {
        /// Сторожевой сервис.
        const QString WatchService = "watchdog";
        /// Обновитель.
        const QString Updater = "updater";
        /// Контроллер сторожевого сервиса.
        const QString WatchServiceController = "controller";
        /// Платёжный процессор.
        const QString PaymentProcessor = "ekiosk";
    } // namespace Modules

    namespace Fields
    {
        /// Модуль.
        const QString Module = "module";
        /// Цель.
        const QString Target = "target";
        /// Отправитель.
        const QString Sender = "sender";
        /// Тип.
        const QString Type = "type";
        /// Параметры.
        const QString Params = "params";
    } // namespace Fields

    namespace States
    {
        enum Enum
        {
            Blocked = 0,      /// Модуль заблокирован
            ServiceOperation, /// Сервисная операция
            ValidatorFailure, /// Ошибка купюроприёмника
            PrinterFailure,   /// Ошибка принтера
            UpdateInProgress, /// Закачка/установка обновления
            NetworkFailure,   /// Ошибка сети
            CryptFailure      /// Ошибка модуля шифрования
        };
    } // namespace States
} // namespace CWatchService

//---------------------------------------------------------------------------
