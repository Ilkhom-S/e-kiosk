/* @file POS-принтер на COM-порту. */

#pragma once

#include <Hardware/Printers/POSPrinter.h>

//--------------------------------------------------------------------------------
/// Последовательный POS-принтер.
template <class T> class SerialPOSPrinter : public T {
public:
    /// Конструктор.
    SerialPOSPrinter() {
        using namespace SDK::Driver::IOPort::COM;

        // параметры порта
        this->m_PortParameters.clear();
        this->m_PortParameters.insert(EParameters::BaudRate,
                                     POSPrinters::TSerialDevicePortParameter()
                                         << EBaudRate::BR115200 << EBaudRate::BR19200
                                         << EBaudRate::BR57600 << EBaudRate::BR38400
                                         << EBaudRate::BR9600 << EBaudRate::BR4800);
        this->m_PortParameters.insert(EParameters::Parity,
                                     POSPrinters::TSerialDevicePortParameter()
                                         << EParity::No << EParity::Even << EParity::Odd);
        this->m_PortParameters.insert(EParameters::ByteSize,
                                     POSPrinters::TSerialDevicePortParameter() << 8);
        this->m_PortParameters.insert(
            EParameters::RTS, POSPrinters::TSerialDevicePortParameter() << ERTSControl::Toggle);
        this->m_PortParameters.insert(
            EParameters::DTR, POSPrinters::TSerialDevicePortParameter() << EDTRControl::Handshake);
    }
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<POSPrinter<TSerialPrinterBase>> TSerialPOSPrinter;
typedef POSPrinter<TLibUSBPrinterBase> TLibUSBPOSPrinter;

//--------------------------------------------------------------------------------
