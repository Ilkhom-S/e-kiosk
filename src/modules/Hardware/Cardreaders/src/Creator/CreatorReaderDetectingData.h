/* @file Данные моделей Creator. */

#pragma once

#include "Hardware/Common/USBDeviceModelData.h"

//--------------------------------------------------------------------------------
namespace CCreatorReader {
/// Название кардридера Creator по умолчанию.
extern const char DefaultName[];

/// Данные модели.
DECLARE_USB_MODEL(DetectingData, Creator, 0x0285, "CRT-288K");
} // namespace CCreatorReader

//--------------------------------------------------------------------------------
