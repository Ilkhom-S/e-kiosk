/* @file Константы кардридеров на протоколе Creator. */

#include "CreatorReaderConstants.h"

/// Команды.
namespace CCreatorReader {
namespace Commands {
const char LockInitialize[] = "\x30\x30";   /// Инициализация с блокировкой карты.
const char UnLockInitialize[] = "\x30\x31"; /// Инициализация с разблокировкой карты.
const char GetSerialNumber[] = "\xA2\x30";  /// Получение серийного номера.
const char GetStatus[] = "\x31\x30";        /// Получение статуса.
const char IdentifyIC[] = "\x50\x30";       /// Автоидентификация IC-карты.
const char IdentifyRF[] = "\x50\x31";       /// Автоидентификация RF-карты.
const char SetMCReadingMode[] =
    "\x36\x30\x30\x31\x37\x30"; /// Установка режима стения карт с магнитной
                                /// полосой, читать все треки в ASCII-формате.
const char ReadMSData[] =
    "\x36\x31\x30\x37"; /// Чтение данных с магнитной полосы, читать все треки в ASCII-формате.
const char PowerReset[] = "\x51\x30\x30"; /// Сброс аппаратный для EMV-карт (cold reset).
const char ADPUT0[] = "\x51\x33";         /// ADPU запрос для карты с протоколом CPU T = 0.
const char ADPUT1[] = "\x51\x34";         /// ADPU запрос для карты с протоколом CPU T = 1.
} // namespace Commands
} // namespace CCreatorReader