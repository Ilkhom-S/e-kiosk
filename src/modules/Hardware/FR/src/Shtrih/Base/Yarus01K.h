/* @file Ярус-01К. */

#pragma once

#include "ShtrihRetractorFR.h"

//--------------------------------------------------------------------------------
class Yarus01K : public ShtrihRetractorFR {
    SET_SUBSERIES("Yarus01K")

public:
    Yarus01K() {
        m_DeviceName = CShtrihFR::Models::CData()[CShtrihFR::Models::ID::Yarus01K].name;
        m_SupportedModels = QStringList() << m_DeviceName;
    }
};

//--------------------------------------------------------------------------------
