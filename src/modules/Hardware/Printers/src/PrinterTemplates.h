/* @file Явное инстанцирование шаблонов принтеров. */

#include <Hardware/Common/LibUSBDeviceBase.h>
#include <Hardware/Common/VirtualDeviceBase.h>

#include "Base/Port/PortPrinterBase.h"
#include "POSPrinters/Citizen/CitizenPPU700/CitizenPPU700.h"
#include "POSPrinters/Common/POSPrinter.h"
#include "POSPrinters/Custom/Custom_Printers.h"
#include "POSPrinters/Custom/Custom_VKP/Custom_VKP80.h"
#include "POSPrinters/Custom/Custom_VKP/Custom_VKP80III.h"
#include "POSPrinters/EjectorPOS/EjectorPOS.h"

template class LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>;
template class PrinterBase<LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>;
template class PortPrinterBase<PrinterBase<LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>>;

template class POSPrinter<TLibUSBPrinterBase>;
template class EjectorPOS<TLibUSBPrinterBase>;
template class Custom_VKP80<TLibUSBPrinterBase>;
template class Custom_Printer<TLibUSBPrinterBase>;
template class Custom_VKP80III<TLibUSBPrinterBase>;
template class CitizenPPU700<TLibUSBPrinterBase>;
template class CitizenPPU700II<TLibUSBPrinterBase>;