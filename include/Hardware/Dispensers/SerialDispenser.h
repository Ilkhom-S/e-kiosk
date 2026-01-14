/* @file Серийный диспенсер. */

// System
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/SerialDeviceBase.h"

// Project
#include "DispenserBase.h"

typedef DispenserBase<SerialDeviceBase<PortPollingDeviceBase<ProtoDispenser>>> TSerialDispenser;

//--------------------------------------------------------------------------------
