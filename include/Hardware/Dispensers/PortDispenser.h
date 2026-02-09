/* @file Диспенсер на порту. */

#pragma once

#include "Hardware/Dispensers/SerialDispenser.h"

//--------------------------------------------------------------------------------
class PortDispenser : public TSerialDispenser {
public:
    PortDispenser() {
        m_ioMessageLogging = ELoggingType::None;
        m_MaxBadAnswers = 2;
        m_PollingInterval = CDispensers::IdlingPollingInterval;
        m_ForceNotWaitFirst = true;
    }
};

//--------------------------------------------------------------------------------
