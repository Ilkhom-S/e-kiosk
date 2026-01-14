/* @file Список компонентов для разработки плагинов. */

#pragma once

namespace SDK {
    namespace PaymentProcessor {

        //---------------------------------------------------------------------------
        /// Все расширения для ПП должны иметь соответствующий идентификатор приложения.
        const char Application[] = "PaymentProcessor";

        //---------------------------------------------------------------------------
        /// Список возможных компонентов, расширяющих функциональность приложения.
        namespace CComponents {
            /// Графический бэкэнд.
            const char GraphicsBackend[] = "GraphicsBackend";
            /// Фабрика платежей.
            const char PaymentFactory[] = "PaymentFactory";
            /// Удалённый клиент.
            const char RemoteClient[] = "RemoteClient";
            /// Графический элемент.
            const char GraphicsItem[] = "GraphicsItem";
            /// Фабрика сценариев.
            const char ScenarioFactory[] = "ScenarioFactory";
            /// Провайдер зарядки.
            const char ChargeProvider[] = "ChargeProvider";
            /// Хук.
            const char Hook[] = "Hook";
            /// Источник рекламы.
            const char AdSource[] = "AdSource";
            /// Фискальный регистратор.
            const char FiscalRegister[] = "FiscalRegister";
            /// Фабрика скриптов.
            const char ScriptFactory[] = "ScriptFactory";
            /// Основной элемент.
            const char CoreItem[] = "CoreItem";
        } // namespace CComponents

        //---------------------------------------------------------------------------
    } // namespace PaymentProcessor
} // namespace SDK

//---------------------------------------------------------------------------
