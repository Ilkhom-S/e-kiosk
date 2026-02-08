/* @file Константы принтеров Swecoin. */

#include "SwecoinPrinterData.h"

//--------------------------------------------------------------------------------
namespace CSwecoinPrinter {
const char UnknownModelName[] = "Unknown Swecoin Printer"; /// Имя принтера по умолчанию.

/// Команды.
namespace Commands {
const char Initialize[] = "\x1B\x40";       /// Инициализация.
const char SetParameter[] = "\x1B\x26\x50"; /// Часть команды установки параметра.
const char GetData[] = "\x1B\x05";          /// Часть команды получения данных об устройстве.
const char GetModelId[] = "\x1B\x05\x63";   /// Получение Id устройства.
const char GetStatus[] = "\x1B\x05\x01";    /// Статус.
const char GetPaperNearEndData[] = "\x1B\x05\x02";
const char GetSensorData[] = "\x1B\x05\x05";
} // namespace Commands
} // namespace CSwecoinPrinter
