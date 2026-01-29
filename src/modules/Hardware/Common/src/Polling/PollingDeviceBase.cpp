/* @file Базовый класс устройств с поллингом. */

// System
#include "Hardware/CardReaders/ProtoMifareReader.h"
#include "Hardware/CashAcceptors/ProtoCashAcceptor.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Dispensers/ProtoDispenser.h"
#include "Hardware/FR/ProtoFR.h"
#include "Hardware/HID/ProtoHID.h"
#include "Hardware/Watchdogs/ProtoWatchdog.h"

// Project
#include "PollingDeviceBase.h"

template class PollingDeviceBase<ProtoPrinter>;
template class PollingDeviceBase<ProtoDispenser>;
template class PollingDeviceBase<ProtoCashAcceptor>;
template class PollingDeviceBase<ProtoFR>;
template class PollingDeviceBase<ProtoHID>;
template class PollingDeviceBase<ProtoMifareReader>;
template class PollingDeviceBase<ProtoDeviceBase>;
template class PollingDeviceBase<ProtoWatchdog>;
