/* @file Константы, коды команд и ответов протокола ФР ПРИМ. */

#include "PrimFRConstants.h"

/// Формат представления даты и времени в ответе на запрос статуса ФН-а.
const char CPrimFR::FRDateTimeFormat[] = "ddMMyyyyhhmm";

/// Количество чеков.
const char CPrimFR::ChecksQuantity[] = "01";

/// ID прошивки для ПРИМ-21 03 на базе принтера Custom VKP-80.
const char CPrimFR::FirmarePRIM21_03[] = "LPC22";

/// Артикул товара/услуги.
const char CPrimFR::ServiceMarking[] = "1";

/// Количество наименований товара/услуги.
const char CPrimFR::ServiceQuantity[] = "1";

/// Единица измерения товара/услуги.
const char CPrimFR::MeasurementUnit[] = " ";

/// Индекс секции.
const char CPrimFR::SectionIndex[] = "01";

/// Идентификатор секции.
const char CPrimFR::SectionID[] = "TERMINAL";

/// Идентификатор (фамилия) оператора.
const char CPrimFR::OperatorID[] = " ";

/// Шрифт.
const char CPrimFR::Font[] = "00";

/// Количество копий.
const char CPrimFR::Copies[] = "01";

/// Шаг сетки строк
const char CPrimFR::LineGrid[] = "00";

/// Название платежной карты. платежных карт у нас нет, но в посылку надо что-то положить.
const char CPrimFR::PaymentCardName[] = " ";

/// Не печатать документ.
const char CPrimFR::DontPrintFD[] = "00";

/// Название налоговой ставки куда будет складываться последний снятый Z отчет.
const char CPrimFR::LastTaxRateName[] = "Z-report number";

/// Версия ПО ФР, где присутствуют презентер и ретрактор.
const char CPrimFR::SoftVersionPresenter[] = "LPC22";

/// Текст на чеке перед выходом из незапланированного режима принтера
const char CPrimFR::EndPrinterModeText[] = "           PRINTER MODE";

/// Команды.
namespace CPrimFR {
namespace Commands {
const char SetFiscalMode[] = "\x1B\x1B";
} // namespace Commands
} // namespace CPrimFR