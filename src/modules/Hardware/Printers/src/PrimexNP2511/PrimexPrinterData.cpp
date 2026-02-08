/* @file Реализация команд для принтера Primex NP2511. */

#include "PrimexPrinterData.h"

/// Команды.
namespace Commands {
const char PrinterInfo[] = "\x1B\x73";
const char GetStatus[] = "\x1B\x76";
const char BackFeed[] = "\x1D\x4A";
const char BackFeed2[] = "\x1B\x42";
const char ClearDispenser[] = "\x1D\x65\x05";
const char Initialize[] = "\x1B\x40";
const char SetCyrillicPage[] = "\x1B\x74\x04";
const char AutoRetract[] = "\x1B\x72\x31\x3A"; // Через 1 минуту засасываем в ретрактор

/// Команды для печати штрих-кода.
namespace BarCode {
const char SetFont[] = "\x1D\x66";
const char SetHeight[] = "\x1D\x68\x80";
const char SetHRIPosition[] = "\x1D\x48\x02";
const char SetWidth[] = "\x1D\x77\x03";
const char Print[] = "\x1D\x6B\x05";
} // namespace BarCode
} // namespace Commands
