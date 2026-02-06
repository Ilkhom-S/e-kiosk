/* @file Базовые статусные типы. */

#pragma once

#include <SDK/Drivers/WarningLevel.h>

#include "Hardware/Common/StatusCache.h"

// TStatusCodes в StatusCache.h;
typedef StatusCache<SDK::Driver::EWarningLevel::Enum> TStatusCollection;

typedef QSet<int> TDeviceCodes;

//--------------------------------------------------------------------------------
