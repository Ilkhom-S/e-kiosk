/* @file Константы принтеров GeBe. */

#include "GeBEData.h"

//--------------------------------------------------------------------------------
namespace CGeBePrinter {
/// Команды.
namespace Commands {
const char Initialize[] = "\x1B\x40";           /// Инициализация.
const char SetFont[] = "\x1B\x50\x31";          /// Установить шрифт.
const char SetLeftAlign[] = "\x1B\x4E\x01\xA0"; /// Установить выравнивание слева.
const char GetStatus[] = "\x1B\x6B\xFF";        /// Cтатус.
} // namespace Commands
} // namespace CGeBePrinter