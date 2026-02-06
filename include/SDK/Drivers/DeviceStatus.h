/* @file Обобщенные статусы устройств. */

#pragma once

namespace SDK {
namespace Driver {

//---------------------------------------------------------------------------
/// Обобщенные статусы устройств. Передаются в пп для уточнения назначения статуса.
namespace EStatus {
enum Enum {
    Actual = 0,     /// Обычный.
    Service = 100,  /// Служебный.
    Interface = 200 /// Интерфейсный.
};
} // namespace EStatus

/// Возвращает тип статуса.
inline EStatus::Enum getStatusType(int aStatus) {
    return (aStatus >= EStatus::Interface)
               ? EStatus::Interface
               : ((aStatus >= EStatus::Service) ? EStatus::Service : EStatus::Actual);
}

} // namespace Driver
} // namespace SDK

//--------------------------------------------------------------------------------
