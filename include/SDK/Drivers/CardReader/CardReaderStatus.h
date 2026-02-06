/* @file Обобщенные статусы кардридеров. */

#pragma once

#include <QtCore/QMetaType>

#include <SDK/Drivers/DeviceStatus.h>

namespace SDK {
namespace Driver {

//---------------------------------------------------------------------------
/// Обобщенные статусы устройств приема денег. Передаются в пп и служат для внутренних нужд
/// драйвера. Порядок не менять.
namespace ECardReaderStatus {
enum Enum {
    OK = 1,  /// Хороший статус.
    Warning, /// Находится в подозрительном состоянии.
    Error,   /// Ошибка.

    /// Нужны для логики драйвера и, возможно, для учета статистики.
    Forgotten = 120, /// Карта забыта.
    NeedReloading,   /// Невозможно прочитать данные карты, нужно повторить.
    Rejected,        /// Выброс - OK.

    /// Нужны для внутреннего пользования в драйверах. Наверх не выдаются.
    SCOperarionError /// Ошибка при работе со смарт-картой.
};
} // namespace ECardReaderStatus

} // namespace Driver
} // namespace SDK

Q_DECLARE_METATYPE(SDK::Driver::ECardReaderStatus::Enum);

//--------------------------------------------------------------------------------
