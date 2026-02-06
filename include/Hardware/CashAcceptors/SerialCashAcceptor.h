/* @file Серийный приемник купюр. */

#include "Hardware/CashAcceptors/PortCashAcceptor.h"
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/SerialDeviceBase.h"

typedef PortCashAcceptor<SerialDeviceBase<PortPollingDeviceBase<ProtoCashAcceptor>>>
    TSerialCashAcceptor;

//--------------------------------------------------------------------------------
