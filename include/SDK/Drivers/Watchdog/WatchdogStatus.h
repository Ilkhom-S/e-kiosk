/* @file Обобщенные статусы сторожевых таймеров. */

#pragma once

#include <QtCore/QMetaType>

#include <SDK/Drivers/DeviceStatus.h>

namespace SDK {
namespace Driver {

//---------------------------------------------------------------------------
/// Обобщенные статусы устройств приема денег. Передаются в пп и служат для внутренних нужд
/// драйвера. Порядок не менять.
namespace EWatchdogStatus {
enum Enum {
    EnterServiceMenu = 140, /// Войти в сервисное меню.
    LockTerminal            /// Заблокировать терминал.
};
} // namespace EWatchdogStatus

} // namespace Driver
} // namespace SDK

Q_DECLARE_METATYPE(SDK::Driver::EWatchdogStatus::Enum);

//--------------------------------------------------------------------------------
