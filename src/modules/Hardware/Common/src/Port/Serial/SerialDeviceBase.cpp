/* @file Базовый класс устройств на COM-порту. */

// System
#include "Hardware/CashAcceptors/ProtoCashAcceptor.h"
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Dispensers/ProtoDispenser.h"
#include "Hardware/FR/ProtoFR.h"
#include "Hardware/HID/ProtoHID.h"
#include "Hardware/IOPorts/IOPortStatusCodes.h"
#include "Hardware/Watchdogs/ProtoWatchdog.h"

// Project
#include "SerialDeviceBase.h"

using namespace SDK::Driver;
using namespace SDK::Driver::IOPort::COM;

//-------------------------------------------------------------------------------
template class SerialDeviceBase<PortPollingDeviceBase<ProtoPrinter>>;
template class SerialDeviceBase<PortPollingDeviceBase<ProtoDispenser>>;
template class SerialDeviceBase<PortPollingDeviceBase<ProtoCashAcceptor>>;
template class SerialDeviceBase<PortPollingDeviceBase<ProtoWatchdog>>;
template class SerialDeviceBase<PortDeviceBase<DeviceBase<ProtoModem>>>;
template class SerialDeviceBase<PortPollingDeviceBase<ProtoFR>>;
template class SerialDeviceBase<PortPollingDeviceBase<ProtoHID>>;

//--------------------------------------------------------------------------------
