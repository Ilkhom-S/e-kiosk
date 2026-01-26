/* @file Серийный диспенсер. */
#pragma once

// System
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/SerialDeviceBase.h"

// Project
#include <Hardware/Dispensers/DispenserBase.h>

typedef DispenserBase<SerialDeviceBase<PortPollingDeviceBase<ProtoDispenser>>> TSerialDispenser;

//--------------------------------------------------------------------------------
