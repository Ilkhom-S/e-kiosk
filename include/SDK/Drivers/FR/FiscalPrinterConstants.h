/* @file Константы фискального регистратора для модуля платежей. */

#pragma once

namespace SDK {
    namespace Driver {

        //--------------------------------------------------------------------------------
        /// Константы фискального регистратора.
        namespace CFiscalPrinter {
            /// Серийный номер.
            const char Serial[] = "serial";
            /// Регистрационный номер.
            const char RNM[] = "rnm";
            /// Номер Z-отчёта.
            const char ZReportNumber[] = "z_report_number";
            /// Количество платежей.
            const char PaymentCount[] = "payment_count";
            /// Необнуляемая сумма.
            const char NonNullableAmount[] = "non_nullable_amount";
            /// Сумма платежа.
            const char PaymentAmount[] = "payment_amount";
            /// Дата и время ФР.
            const char FRDateTime[] = "fr_date_time";
            /// Системная дата и время.
            const char SystemDateTime[] = "system_date_time";
        } // namespace CFiscalPrinter

    } // namespace Driver
} // namespace SDK

//--------------------------------------------------------------------------------
