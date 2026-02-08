/* @file Константы фискального регистратора для модуля платежей. */

#pragma once

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------
/// Константы фискального регистратора.
namespace CFiscalPrinter {
/// Серийный номер.
extern const char Serial[];
/// Регистрационный номер.
extern const char RNM[];
/// Номер Z-отчёта.
extern const char ZReportNumber[];
/// Количество платежей.
extern const char PaymentCount[];
/// Необнуляемая сумма.
extern const char NonNullableAmount[];
/// Сумма платежа.
extern const char PaymentAmount[];
/// Дата и время ФР.
extern const char FRDateTime[];
/// Системная дата и время.
extern const char System_DateTime[];
} // namespace CFiscalPrinter

} // namespace Driver
} // namespace SDK

//--------------------------------------------------------------------------------
