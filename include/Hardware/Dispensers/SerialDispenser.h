/* @file Серийный диспенсер. */
#pragma once

#include <Hardware/Dispensers/DispenserBase.h>

#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/SerialDeviceBase.h"

typedef DispenserBase<SerialDeviceBase<PortPollingDeviceBase<ProtoDispenser>>> TSerialDispenser;

//--------------------------------------------------------------------------------
