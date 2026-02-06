/* @file Принтер Citizen CT-S310II. */

#pragma once

#include "CitizenBase.h"

//--------------------------------------------------------------------------------
class CitizenCTS310II : public CitizenBase<TSerialPOSPrinter> {
    SET_SUBSERIES("CitizenCTS310II")

public:
    CitizenCTS310II();
};

//--------------------------------------------------------------------------------
