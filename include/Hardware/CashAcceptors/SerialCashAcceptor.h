/* @file Серийный приемник купюр. */

// System
#include "Hardware/CashAcceptors/PortCashAcceptor.h"
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/SerialDeviceBase.h"

typedef PortCashAcceptor<SerialDeviceBase<PortPollingDeviceBase<ProtoCashAcceptor>>> TSerialCashAcceptor;

//--------------------------------------------------------------------------------