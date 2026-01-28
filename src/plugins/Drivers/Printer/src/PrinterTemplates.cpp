/* @file Инстанцирование шаблонов принтеров. */

// System
#include <Hardware/Common/LibUSBDeviceBase.h>
#include <Hardware/Printers/POSPrinter.h>
#include <Hardware/Printers/PrinterDevices.h>
#include <POSPrinters/Citizen/CitizenPPU700/CitizenPPU700.h>
#include <POSPrinters/Custom/CustomPrinters.h>
#include <POSPrinters/Custom/CustomVKP/CustomVKP80.h>
#include <POSPrinters/EjectorPOS/EjectorPOS.h>

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

//------------------------------------------------------------------------------
