/* @file Явное инстанцирование шаблонов принтеров. */

// System
#include "Base/Port/PortPrinterBase.h"
#include <Hardware/Common/LibUSBDeviceBase.h>
#include <Hardware/Common/VirtualDeviceBase.h>
#include "POSPrinters/Citizen/CitizenPPU700/CitizenPPU700.h"
#include "POSPrinters/Common/POSPrinter.h"
#include "POSPrinters/Custom/CustomPrinters.h"
#include "POSPrinters/Custom/CustomVKP/CustomVKP80.h"
#include "POSPrinters/Custom/CustomVKP/CustomVKP80III.h"
#include "POSPrinters/EjectorPOS/EjectorPOS.h"

template class LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>;
template class PrinterBase<LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>;
template class PortPrinterBase<PrinterBase<LibUSBDeviceBase<PortPollingDeviceBase<ProtoPrinter>>>>;

template class POSPrinter<TLibUSBPrinterBase>;
template class EjectorPOS<TLibUSBPrinterBase>;
template class CustomVKP80<TLibUSBPrinterBase>;
template class CustomPrinter<TLibUSBPrinterBase>;
template class CustomVKP80III<TLibUSBPrinterBase>;
template class CitizenPPU700<TLibUSBPrinterBase>;
template class CitizenPPU700II<TLibUSBPrinterBase>;