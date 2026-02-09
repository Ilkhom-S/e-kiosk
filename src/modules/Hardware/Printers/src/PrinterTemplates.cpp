/* @file Явное инстанцирование шаблонов принтеров. */

// Include template implementations for explicit instantiation

#include "PrinterTemplates.h"

#include <Hardware/Common/VirtualDeviceBase.h>

#include "Base/Port/PortPrinterBase.cpp"
#include "POSPrinters/Citizen/CitizenPPU700/CitizenPPU700.cpp"
#include "POSPrinters/Common/POSPrinter.cpp"
#include "POSPrinters/Custom/CustomPrinters.cpp"
#include "POSPrinters/Custom/CustomVKP/CustomVKP80.cpp"
#include "POSPrinters/Custom/CustomVKP/CustomVKP80III.cpp"
#include "POSPrinters/EjectorPOS/EjectorPOS.cpp"

// Explicit template instantiations
template class Custom_VKP80<TLibUSBPrinterBase>;
template class CitizenPPU700<TLibUSBPrinterBase>;
template class CitizenPPU700II<TLibUSBPrinterBase>;
