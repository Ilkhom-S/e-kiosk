/* @file Константы, коды команд и ответов протокола ФР АТОЛ онлайн. */

#include "AtolOnlineFRConstants.h"

/// Имя модели по умолчанию
const char CAtolOnlineFR::DefaultModelName[] = "ATOL online FR";

/// Формат представления даты [активизации] ФН в ответе на длинный запрос статуса ФН.
const char CAtolOnlineFR::DateFormat[] = "yyyyMMdd";

/// Формат представления даты для вывода в лог.
const char CAtolOnlineFR::TimeLogFormat[] = "hh:mm";

/// Регистры
namespace CAtolOnlineFR {
namespace Registers {
const char OFDNotSentCount[] = "not sent fiscal to OFD documents count";
const char ExtendedErrorData[] = "extended error data";
const char FFD[] = "FFD";
const char SessionData[] = "session data";
} // namespace Registers

/// Команды.
namespace Commands {
extern const char Reboot[] = "\xCE\x41";              /// Перезагрузить ФР по питанию.
extern const char GetInternalFirmware[] = "\x9D\x91"; /// Получить внутренний номер прошивки.

/// Коды команд ФН-а.
namespace FS {
const char GetStatus[] = "\xA4\x30";             /// Получить статус ФН.
const char GetNumber[] = "\xA4\x31";             /// Получить номер ФН.
const char GetValidity[] = "\xA4\x32";           /// Получить срок действия ФН.
const char GetVersion[] = "\xA4\x33";            /// Получить версию ФН.
const char GetFiscalizationTotal[] = "\xA4\x43"; /// Получить итог фискализации.
const char GetFDbyNumber[] = "\xA4\x40";         /// Получить фискальный документ по его номеру.
const char StartFiscalTLVData[] =
    "\xA4\x45"; /// Начать получение данных фискального документа в TLV-формате.
const char GetFiscalTLVData[] = "\xA4\x46"; /// Получить данные фискального документа в TLV-формате.
} // namespace FS
} // namespace Commands
} // namespace CAtolOnlineFR
