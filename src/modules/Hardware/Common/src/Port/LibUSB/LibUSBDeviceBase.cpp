/* @file Определение статических членов LibUSBDeviceBase. */

#include "LibUSBDeviceBase.h"

#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"

template class LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>;
