/* @file Список компонентов для разработки плагинов. */

#pragma once

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
/// Все расширения для ПП должны иметь соответствующий идентификатор приложения.
extern const char Application[];

//---------------------------------------------------------------------------
/// Список возможных компонентов, расширяющих функциональность приложения.
namespace CComponents {
/// Графический бэкэнд.
extern const char GraphicsBackend[];
/// Фабрика платежей.
extern const char PaymentFactory[];
/// Удалённый клиент.
extern const char RemoteClient[];
/// Графический элемент.
extern const char GraphicsItem[];
/// Фабрика сценариев.
extern const char ScenarioFactory[];
/// Провайдер зарядки.
extern const char ChargeProvider[];
/// Хук.
extern const char Hook[];
/// Источник рекламы.
extern const char AdSource[];
/// Фискальный регистратор.
extern const char FiscalRegister[];
/// Фабрика скриптов.
extern const char ScriptFactory[];
/// Основной элемент.
extern const char CoreItem[];
} // namespace CComponents

//---------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK

//---------------------------------------------------------------------------
