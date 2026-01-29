/* @file Инстанцирование шаблонов виртуальных устройств. */

// System
#include "../../../../modules/Hardware/CashAcceptors/src/CashAcceptorBase.cpp"
#include "../../../../modules/Hardware/CashDispensers/src/DispenserBase.cpp"
#include <Hardware/CashAcceptors/CashAcceptorBase.h>
#include <Hardware/Common/DeviceBase.h>
#include <Hardware/Common/PollingDeviceBase.h>
#include <Hardware/Common/ProtoDevices.h>
#include <Hardware/Common/VirtualDeviceBase.h>
#include <Hardware/Dispensers/DispenserBase.h>
#include <Hardware/Printers/PrinterBase.h>

template class CashAcceptorBase<DeviceBase<ProtoCashAcceptor>>;
template class DispenserBase<DeviceBase<ProtoDispenser>>;
template class PollingDeviceBase<ProtoPrinter>;
template class PrinterBase<PollingDeviceBase<ProtoPrinter>>;
template class VirtualDeviceBase<PrinterBase<PollingDeviceBase<ProtoPrinter>>>;

//------------------------------------------------------------------------------
