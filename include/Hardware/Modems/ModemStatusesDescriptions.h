/* @file Описатель кодов состояний модемов. */

#pragma once

#include "Hardware/Common/BaseStatusDescriptions.h"
#include "Hardware/Modems/ModemStatusCodes.h"

//--------------------------------------------------------------------------------
namespace Modem_StatusCode {
/// Спецификации кодов состояний модемов.
class CSpecifications : public DeviceStatusCode::CSpecifications {
public:
    /// Конструктор.
    CSpecifications() {
        ADD_ERROR_STATUS(SIMError, QCoreApplication::translate("Modem_Statuses", "#SIM"));
        ADD_ERROR_STATUS(NoNetwork, QCoreApplication::translate("Modem_Statuses", "#NoNetwork"));
    }
};
} // namespace Modem_StatusCode

//--------------------------------------------------------------------------------
