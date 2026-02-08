/* @file Сторожевой таймер STOD. */

#pragma once

#include <Hardware/Watchdogs/OSMP.h>

//--------------------------------------------------------------------------------
class STOD : public OSMP {
    SET_SERIES("STOD")

public:
    STOD() {
        m_DeviceName = "STOD";

        m_Data[EOSMPCommandId::IdentificationData] = "STODSIM";

        m_Data[EOSMPCommandId::Identification] = "OSP\x09";
        m_Data[EOSMPCommandId::RebootPC] = "OSTR";
    }
};

//--------------------------------------------------------------------------------