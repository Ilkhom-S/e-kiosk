/* @file Определение статических членов LibUSBDeviceBase. */

// System
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"

// Project
#include "LibUSBDeviceBase.h"

template class LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>;
