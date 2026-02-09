/* @file Константы принтеров Star. */

#include "StarPrinterData.h"
#include <Hardware/Printers/StarModelData.h>

/// Команды.
namespace CStarPrinter {
namespace Commands {
const char Initialize[] = "\x1B\x40";        /// Переинициализация логики.
const char Reset[] = "\x1B\x06\x18";         /// Сброс (логика + механика).
const char ASBStatus[] = "\x1B\x06\x01";     /// Статус (ASB).
const char ETBMark[] = "\x17";               /// Метка (ETB) для ASB статуса.
const char SetASB[] = "\x1B\x1E\x61\x03";    /// Установить ASB.
const char PrintImage[] = "\x1B\x58";        /// Печать изображения.
const char FeedImageLine[] = "\x1B\x49\x18"; /// Промотка линии изображения.
} // namespace Commands
} // namespace CStarPrinter

//--------------------------------------------------------------------------------
namespace CSTAR {
namespace Models {
const char TUP542[] = "STAR TUP542";
const char TUP592[] = "STAR TUP592";
const char TUP942[] = "STAR TUP942";
const char TUP992[] = "STAR TUP992";
const char TSP613[] = "STAR TSP613";
const char TSP643[] = "STAR TSP643";
const char TSP651[] = "STAR TSP651";
const char TSP654[] = "STAR TSP654";
const char TSP654II[] = "STAR TSP654II";
const char TSP743[] = "STAR TSP743";
const char TSP743II[] = "STAR TSP743II";
const char TSP847[] = "STAR TSP847";
const char TSP847II[] = "STAR TSP847II";
const char TSP828L[] = "STAR TSP828L";
const char TSP1043[] = "STAR TSP1043";
const char FVP10[] = "STAR FVP10";

const char Unknown[] = "Unknown STAR printer";
const char UnknownEjector[] = "Unknown STAR printer with ejector";
} // namespace Models
} // namespace CSTAR

//--------------------------------------------------------------------------------
CSTAR::Models::CData CSTAR::Models::Data;
