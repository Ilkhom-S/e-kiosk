/* @file Инстанцирование PrinterBase. */

// System
#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Common/SerialDeviceBase.h"

// Project
#include "PrinterBase.h"

using namespace SDK::Driver;

// Явное инстанцирование для поддерживаемых типов (обязательно для Multiplatform)
template class PrinterBase<PollingDeviceBase<ProtoPrinter>>;
template class PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>;
