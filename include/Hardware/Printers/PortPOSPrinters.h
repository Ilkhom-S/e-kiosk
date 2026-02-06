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
        this->mPortParameters.clear();
        this->mPortParameters.insert(EParameters::BaudRate,
                                     POSPrinters::TSerialDevicePortParameter()
                                         << EBaudRate::BR115200 << EBaudRate::BR19200
                                         << EBaudRate::BR57600 << EBaudRate::BR38400
                                         << EBaudRate::BR9600 << EBaudRate::BR4800);
        this->mPortParameters.insert(EParameters::Parity,
                                     POSPrinters::TSerialDevicePortParameter()
                                         << EParity::No << EParity::Even << EParity::Odd);
        this->mPortParameters.insert(EParameters::ByteSize,
                                     POSPrinters::TSerialDevicePortParameter() << 8);
        this->mPortParameters.insert(
            EParameters::RTS, POSPrinters::TSerialDevicePortParameter() << ERTSControl::Toggle);
        this->mPortParameters.insert(
            EParameters::DTR, POSPrinters::TSerialDevicePortParameter() << EDTRControl::Handshake);
    }
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<POSPrinter<TSerialPrinterBase>> TSerialPOSPrinter;
typedef POSPrinter<TLibUSBPrinterBase> TLibUSBPOSPrinter;

//--------------------------------------------------------------------------------
