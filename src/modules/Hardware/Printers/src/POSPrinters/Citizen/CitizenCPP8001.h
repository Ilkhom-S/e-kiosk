/* @file Принтер Citizen CPP-8001. */

#pragma once

#include "CitizenBase.h"

//--------------------------------------------------------------------------------
class CitizenCPP8001 : public CitizenBase<TSerialPOSPrinter> {
    SET_SUBSERIES("CitizenCPP8001")

public:
    CitizenCPP8001();

    /// Устанавливает конфигурацию устройству.
    virtual void setDeviceConfiguration(const QVariantMap &aConfiguration);
};

//--------------------------------------------------------------------------------
