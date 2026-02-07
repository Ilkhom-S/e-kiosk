/* @file Определения констант компонентов. */

#include <SDK/PaymentProcessor/Components.h>

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
/// Все расширения для ПП должны иметь соответствующий идентификатор приложения.
extern const char Application[] = "PaymentProcessor";

//---------------------------------------------------------------------------
/// Список возможных компонентов, расширяющих функциональность приложения.
namespace CComponents {
/// Графический бэкэнд.
extern const char GraphicsBackend[] = "GraphicsBackend";
/// Фабрика платежей.
extern const char PaymentFactory[] = "PaymentFactory";
/// Удалённый клиент.
extern const char RemoteClient[] = "RemoteClient";
/// Графический элемент.
extern const char GraphicsItem[] = "GraphicsItem";
/// Фабрика сценариев.
extern const char ScenarioFactory[] = "ScenarioFactory";
/// Провайдер зарядки.
extern const char ChargeProvider[] = "ChargeProvider";
/// Хук.
extern const char Hook[] = "Hook";
/// Источник рекламы.
extern const char AdSource[] = "AdSource";
/// Фискальный регистратор.
extern const char FiscalRegister[] = "FiscalRegister";
/// Фабрика скриптов.
extern const char ScriptFactory[] = "ScriptFactory";
/// Основной элемент.
extern const char CoreItem[] = "CoreItem";
} // namespace CComponents

//---------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
