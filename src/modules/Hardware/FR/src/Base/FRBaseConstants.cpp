/* @file Общие константы ФР. */

#include <Hardware/FR/FRBaseConstants.h>

//--------------------------------------------------------------------------------
/// Общие константы ФР.
namespace CFR {
/// Формат представления даты.
const char DateFormat[] = "ddMMyyyy";

/// Формат представления даты-времени для парсинга даты bp TLV-параметра фискального тега.
const char TLVDateTimeFormat[] = "yyyyMMddhhmm";

/// Формат представления даты для вывода в лог.
const char DateLogFormat[] = "dd.MM.yyyy";

/// Формат представления даты для вывода в лог.
const char TimeLogFormat[] = "hh:mm:ss";

/// Формат представления даты для вывода в лог даты и времени.
const char DateTimeLogFormat[] = "dd.MM.yyyy hh:mm:ss";

/// Формат представления даты для вывода в лог даты и времени.
const char DateTimeShortLogFormat[] = "dd.MM.yyyy hh:mm";

/// Результаты запроса статуса.
namespace Result {
const char Error[] = "__ERROR__"; /// Ошибка устройства, либо ответ неверно скомпонован.
const char Fail[] = "__FAIL__";   /// Транспортная/протокольная ошибка.
} // namespace Result

/// Операция платежного агента (1044).
namespace AgentOperation {
const char Payment[] = "Платеж";
const char Payout[] = "Выдача наличных";
} // namespace AgentOperation
} // namespace CFR

//--------------------------------------------------------------------------------