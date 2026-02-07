/* @file Константы принтеров Fujitsu. */

#include "FujitsuPrinterData.h"

//--------------------------------------------------------------------------------
namespace CFujitsuPrinter {
/// Команды.
namespace Commands {
const char Identification[] = "\x17";
const char Initialize[] = "\x16";
const char Status[] = "\x18";
const char Voltage[] = "\x19";
} // namespace Commands
} // namespace CFujitsuPrinter