/* @file Данные протокола AT. */

#pragma once

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний устройств на протоколе AT.
namespace AT {
namespace EModemDialect {
enum Enum { DefaultAtGsm, Siemens, SimCom, Huawei, ZTE };
} // namespace EModemDialect

/// Команды.
namespace Commands {
extern const char AT[];   /// Префикс AT добавляется автоматически.
extern const char ATZ[]; /// Сброс конфигурации.
extern const char ATE[];
extern const char ATI[];
extern const char ATandF0[];
extern const char ATandV[];
extern const char Revision[];       /// Получение ревизии модема
extern const char IMEI[];           /// Запрос получения IMEI модема
extern const char IMSI[];          /// Запрос получения IMSI
extern const char CPIN[];         /// Запрос состояния SIM карты
extern const char CNUM[];          /// Запрос номера телефона симки
extern const char CopsMode[];  /// Переключение режима вывода имени оператора
extern const char COPS[];         /// Запрос имени оператора.
extern const char CSQ[];            /// Запрос качества сигнала.
extern const char CUSD[];        /// Отправка USSD запроса.
extern const char CREG[];         /// Проверка доступности провайдера.
extern const char SetTextMode[]; /// Текстовый режим при отправке SMS.
extern const char SetPDUMode[];  /// Бинарный режим работы с SMS.
extern const char SendSMS[];      /// Отправка SMS.
extern const char ListSMS[];     /// Получение списка всех SMS.
extern const char DeleteSMS[];    /// Удаление SMS.
const char StrgZ = 0x1A;              /// ^Z, символ, завершающий отправку SMS.

namespace Huawei {
extern const char SIMID[]; /// SIM card identification number
} // namespace Huawei

namespace Siemens {
extern const char Restart[]; /// Перегрузить модем и заставить его перерегистрироваться в
                                    /// сети. Требует 5 секундного ожидания.
extern const char SIMID[];            /// SIM card identification number
extern const char GetCellList[];     /// получение списка базовых станций
extern const char ActiveCellInfo[];   /// получение уровня сигнала активной базовой станции
extern const char InactiveCellInfo[]; /// получение уровня сигнала не активных базовых станции
} // namespace Siemens

namespace ZTE {
extern const char SIMID[]; /// SIM card identification number
extern const char PowerOn[];              /// Подать питание на GSM модуль
} // namespace ZTE

namespace SimCom {
extern const char Restart[]; /// Перегрузить модем и заставить его перерегистрироваться в
                                    /// сети. Требует 5 секундного ожидания.
extern const char CSCS[]; /// Переключение кодировки USSD запросов.

extern const char CGREG[];        /// Network Registration Status
extern const char GetCellList[]; /// получение списка базовых станций
} // namespace SimCom
} // namespace Commands
} // namespace AT

//--------------------------------------------------------------------------------
