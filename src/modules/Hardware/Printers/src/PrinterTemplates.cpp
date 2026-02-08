/* @file Явное инстанцирование шаблонов принтеров. */

// Include template implementations for explicit instantiation

#include "PrinterTemplates.h"

#include <Hardware/Common/VirtualDeviceBase.h>

#include "Base/Port/PortPrinterBase.cpp"
#include "POSPrinters/Citizen/CitizenPPU700/CitizenPPU700.cpp"
#include "POSPrinters/Common/POSPrinter.cpp"
#include "POSPrinters/Custom/Custom_Printers.cpp"
#include "POSPrinters/Custom/Custom_VKP/Custom_VKP80.cpp"
#include "POSPrinters/Custom/Custom_VKP/Custom_VKP80III.cpp"
#include "POSPrinters/EjectorPOS/EjectorPOS.cpp"