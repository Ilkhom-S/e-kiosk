/* @file Помощник по логике работы устройства. */

#pragma once

#include <Common/ILogable.h>

#include "DeviceConfigManager.h"
#include "DeviceLogManager.h"

//--------------------------------------------------------------------------------
/// Обобщенные состояния выполнения запроса.
namespace ERequestStatus {
enum Enum { Success = 0, InProcess, Fail };
} // namespace ERequestStatus

//--------------------------------------------------------------------------------
class DeviceLogicManager : public DeviceLogManager, public DeviceConfigManager {};

//--------------------------------------------------------------------------------
