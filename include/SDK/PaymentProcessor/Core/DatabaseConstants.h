/* @file Имена параметров в БД. */

#pragma once

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
namespace CDatabaseConstants {
/// Имена обязательных устройств.
namespace Devices {
extern const char Terminal[];
} // namespace Devices

/// Имена параметров в БД.
namespace Parameters {
/// Общие параметры.
extern const char DeviceName[]; /// Содержит имя устройства.
extern const char DeviceInfo[]; /// Содержит различную информацию о устройстве.

/// Параметры принтеров.
extern const char ReceiptCount[]; /// Счётчик напечатанных чеков.

/// Параметры модемов.
extern const char BalanceLevel[];   /// Баланс сим-карты.
extern const char SignalLevel[];    /// Уровень сигнала модема.
extern const char ConnectionName[]; /// Название модемного соединения.
extern const char LastCheckBalanceTime[];
/// Время последней проверки баланса и уровня сигнала

/// Параметры купюроприемника.
extern const char RejectCount[];

/// Параметры терминала.
extern const char DisabledParam[];     /// Заблокирован или нет терминал.
extern const char LastUpdateTime[];    /// Время последнего скачивания конфигураций.
extern const char LaunchCount[];       /// Число запусков ПО.
extern const char LastStartDate[];     /// Время последнего запуска.
extern const char Configuration[];     /// Конфигурация ПО.
extern const char OperationSystem[];   /// Версия операционной системы.
extern const char DisplayResolution[]; /// Разрешение экрана.

/// Параметры диспенсера
extern const char CashUnits[]; /// Описание содержимого диспенсера по кассетам
} // namespace Parameters
} // namespace CDatabaseConstants

//---------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK
