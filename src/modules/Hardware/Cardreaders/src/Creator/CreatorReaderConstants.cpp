/* @file Константы кардридеров на протоколе Creator. */

#include "CreatorReaderConstants.h"

/// Команды.
namespace CCreatorReader {
namespace Commands {
const char LockInitialize[] = R"(00)";      /// Инициализация с блокировкой карты.
const char UnLockInitialize[] = R"(01)";    /// Инициализация с разблокировкой карты.
const char GetSerialNumber[] = "\xA2\x30";  /// Получение серийного номера.
const char GetStatus[] = R"(10)";           /// Получение статуса.
const char IdentifyIC[] = R"(P0)";          /// Автоидентификация IC-карты.
const char IdentifyRF[] = R"(P1)";          /// Автоидентификация RF-карты.
const char SetMCReadingMode[] = R"(600170)"; /// Установка режима стения карт с магнитной
                                             /// полосой, читать все треки в ASCII-формате.
const char ReadMSData[] =
    R"(6107)"; /// Чтение данных с магнитной полосы, читать все треки в ASCII-формате.
const char PowerReset[] = R"(Q00)"; /// Сброс аппаратный для EMV-карт (cold reset).
const char ADPUT0[] = R"(Q3)";      /// ADPU запрос для карты с протоколом CPU T = 0.
const char ADPUT1[] = R"(Q4)";      /// ADPU запрос для карты с протоколом CPU T = 1.
} // namespace Commands
} // namespace CCreatorReader
