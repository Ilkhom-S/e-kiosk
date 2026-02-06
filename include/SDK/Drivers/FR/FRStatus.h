/* @file Обобщенные статусы фискальных регистраторов. */

#pragma once

#include <QtCore/QMetaType>

#include <SDK/Drivers/DeviceStatus.h>

namespace SDK {
namespace Driver {

//---------------------------------------------------------------------------
/// Статусы фискальных регистраторов.
namespace EFRStatus {
enum Enum {
    NoMoneyForSellingBack = 160 /// Не хватает денег для возврата товара.
};
} // namespace EFRStatus

} // namespace Driver
} // namespace SDK

Q_DECLARE_METATYPE(SDK::Driver::EFRStatus::Enum);

//--------------------------------------------------------------------------------
