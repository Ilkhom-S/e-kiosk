/* @file Константы, коды команд и ответов протокола ФР ПРИМ. */

#include "Prim_FRConstants.h"

/// Формат представления даты и времени в ответе на запрос статуса ФН-а.
const char CPrim_FR::FRDateTimeFormat[] = "ddMMyyyyhhmm";

/// Количество чеков.
const char CPrim_FR::ChecksQuantity[] = "01";

/// ID прошивки для ПРИМ-21 03 на базе принтера Custom VKP-80.
const char CPrim_FR::FirmarePRIM21_03[] = "LPC22";

/// Артикул товара/услуги.
const char CPrim_FR::ServiceMarking[] = "1";

/// Количество наименований товара/услуги.
const char CPrim_FR::ServiceQuantity[] = "1";

/// Единица измерения товара/услуги.
const char CPrim_FR::MeasurementUnit[] = " ";

/// Индекс секции.
const char CPrim_FR::SectionIndex[] = "01";

/// Идентификатор секции.
const char CPrim_FR::SectionID[] = "TERMINAL";

/// Идентификатор (фамилия) оператора.
const char CPrim_FR::OperatorID[] = " ";

/// Шрифт.
const char CPrim_FR::Font[] = "00";

/// Количество копий.
const char CPrim_FR::Copies[] = "01";

/// Шаг сетки строк
const char CPrim_FR::LineGrid[] = "00";

/// Название платежной карты. платежных карт у нас нет, но в посылку надо что-то положить.
const char CPrim_FR::PaymentCardName[] = " ";

/// Не печатать документ.
const char CPrim_FR::DontPrintFD[] = "00";

/// Название налоговой ставки куда будет складываться последний снятый Z отчет.
const char CPrim_FR::LastTaxRateName[] = "Z-report number";

/// Версия ПО ФР, где присутствуют презентер и ретрактор.
const char CPrim_FR::SoftVersionPresenter[] = "LPC22";

/// Текст на чеке перед выходом из незапланированного режима принтера
const char CPrim_FR::EndPrinterModeText[] = "           PRINTER MODE";

/// Команды.
namespace CPrim_FR {
namespace Commands {
const char SetFiscalMode[] = "\x1B\x1B";
} // namespace Commands
} // namespace CPrim_FR
