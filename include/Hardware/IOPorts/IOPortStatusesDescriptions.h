/* @file Описатель общих кодов состояний портов. */

#pragma once

#include "Hardware/Common/BaseStatusDescriptions.h"
#include "Hardware/IOPorts/IOPortStatusCodes.h"

//--------------------------------------------------------------------------------
namespace IOPortStatusCode
{
    /// Спецификации кодов состояний портов.
    class CSpecifications : public DeviceStatusCode::CSpecifications
    {
      public:
        /// Конструктор.
        CSpecifications()
        {
            /// Предупреждения.
            ADD_WARNING_STATUS(MismatchParameters,
                               QCoreApplication::translate("IOPortStatuses", "#mismatch_parameters"));

            /// Ошибки.
            ADD_ERROR_STATUS(NotConnected, QCoreApplication::translate("IOPortStatuses", "#not_connected"));
            ADD_ERROR_STATUS(NotSet, QCoreApplication::translate("IOPortStatuses", "#not_set"));
            ADD_ERROR_STATUS(Busy, QCoreApplication::translate("IOPortStatuses", "#busy"));
            ADD_ERROR_STATUS(NotConfigured, QCoreApplication::translate("IOPortStatuses", "#not_configured"));
        }
    };
} // namespace IOPortStatusCode

//--------------------------------------------------------------------------------
