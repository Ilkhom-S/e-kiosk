/* @file Явное инстанцирование шаблонов принтеров. */

#include <Hardware/Common/LibUSBDeviceBase.h>
#include <Hardware/Common/VirtualDeviceBase.h>
#include <Hardware/Printers/CustomVKP80.h>
#include <Hardware/Printers/POSPrinter.h>

#include "Base/Port/PortPrinterBase.h"
#include "POSPrinters/Citizen/CitizenPPU700/CitizenPPU700.h"
#include "POSPrinters/Custom/CustomPrinters.h"
#include "POSPrinters/Custom/CustomVKP/CustomVKP80III.h"
#include "POSPrinters/EjectorPOS/EjectorPOS.h"

template class LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>;
template class PrinterBase<LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>;
template class PortPrinterBase<PrinterBase<LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>>;

template class POSPrinter<TLibUSBPrinterBase>;
template class EjectorPOS<TLibUSBPrinterBase>;
