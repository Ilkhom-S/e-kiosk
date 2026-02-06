/* @file Инстанцирование PrinterBase. */

#include "PrinterBase.h"

#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/ProtoDevices.h"
#include "Hardware/Common/SerialDeviceBase.h"

using namespace SDK::Driver;

// Явное инстанцирование для поддерживаемых типов (обязательно для Multiplatform)
template class PrinterBase<PollingDeviceBase<ProtoPrinter>>;
template class PrinterBase<SerialDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>;
