/* @file Описатель кодов состояний модемов. */

#pragma once

#include "Hardware/Common/BaseStatusDescriptions.h"
#include "Hardware/Modems/ModemStatusCodes.h"

//--------------------------------------------------------------------------------
namespace ModemStatusCode
{
    /// Спецификации кодов состояний модемов.
    class CSpecifications : public DeviceStatusCode::CSpecifications
    {
      public:
        /// Конструктор.
        CSpecifications()
        {
            ADD_ERROR_STATUS(SIMError, QCoreApplication::translate("ModemStatuses", "#SIM"));
            ADD_ERROR_STATUS(NoNetwork, QCoreApplication::translate("ModemStatuses", "#NoNetwork"));
        }
    };
} // namespace ModemStatusCode

//--------------------------------------------------------------------------------
