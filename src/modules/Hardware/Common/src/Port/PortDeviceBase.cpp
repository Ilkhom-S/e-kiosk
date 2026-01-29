/* @file Базовый класс устройств на порту. */

// System
#include "Hardware/CardReaders/ProtoMifareReader.h"
#include "Hardware/CashAcceptors/ProtoCashAcceptor.h"
#include "Hardware/Common/PollingDeviceBase.h"
#include <Hardware/Common/PortDeviceBase.h>
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Dispensers/ProtoDispenser.h"
#include "Hardware/FR/ProtoFR.h"
#include "Hardware/HID/ProtoHID.h"
#include "Hardware/Watchdogs/ProtoWatchdog.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
// Explicit template instantiations
template class PortDeviceBase<PollingDeviceBase<ProtoPrinter>>;
template class PortDeviceBase<PollingDeviceBase<ProtoDispenser>>;
template class PortDeviceBase<PollingDeviceBase<ProtoCashAcceptor>>;
template class PortDeviceBase<PollingDeviceBase<ProtoWatchdog>>;
template class PortDeviceBase<DeviceBase<ProtoModem>>;
template class PortDeviceBase<PollingDeviceBase<ProtoFR>>;
template class PortDeviceBase<PollingDeviceBase<ProtoMifareReader>>;
template class PortDeviceBase<PollingDeviceBase<ProtoHID>>;

//--------------------------------------------------------------------------------
