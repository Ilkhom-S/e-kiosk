# Hardware Module

## Purpose

The Hardware module provides comprehensive device abstraction and communication layers for kiosk hardware peripherals including fiscal registers, card readers, cash acceptors/dispensers, printers, modems, and scanners. It offers unified APIs for device management, protocol handling, and cross-platform hardware integration.

---

## Quick start 🔧

```cpp
#include "Hardware/Common/DeviceBase.h"
#include "Hardware/IOPorts/SerialPort.h"

// Basic device initialization
class MyDevice : public DeviceBase {
    SerialPort* m_port;

    bool initialize() override {
        m_port = new SerialPort("COM1", 9600);
        return m_port->open();
    }
};
```

---

## Features

- **Device Abstraction**: Unified base classes (`DeviceBase`, `PortDevice`, `USBDevice`) for consistent device lifecycle management
- **Communication Layers**: Serial ports, USB HID, TCP/IP ports with libusb integration
- **Fiscal Registers**: Support for multiple FR protocols (Atol, Shtrih, Prim, Spark, Kasbi, MStar)
- **Payment Devices**: Card readers (IDTech SDK), cash acceptors/dispensers, coin acceptors
- **Printers**: Thermal receipt printing with barcode/QR code generation
- **Modems**: AT command-based modem communication with SMS support
- **Scanners**: Serial-based barcode scanners
- **Watchdogs**: Hardware watchdog timer management
- **Protocol Abstraction**: Modular protocol implementations for different device types
- **Error Handling**: Comprehensive status codes and error reporting
- **Thread Safety**: Designed for concurrent device operations
- **Configuration**: SettingsManager integration for device parameters

---

## Platform support

| Platform | Status     | Notes                                                            |
| -------- | ---------- | ---------------------------------------------------------------- |
| Windows  | ✅ Full    | Full hardware support; USB/serial ports; fiscal registers        |
| Linux    | 🔬 Partial | Serial/USB ports supported; fiscal registers need testing        |
| macOS    | 🔬 Partial | Serial/USB ports supported; limited hardware device availability |

---

## Configuration

Hardware devices are configured through the `SettingsManager`. Common settings sections:

```ini
[Hardware]
DefaultTimeout=5000
RetryCount=3
LogLevel=Info

[Hardware/FR]
Model=Atol
Port=COM1
BaudRate=115200

[Hardware/CardReader]
Type=IDTech
Port=COM2

[Hardware/Printer]
Type=Thermal
Port=COM3
```

---

## Usage / API highlights

### Device Base Classes

- `DeviceBase` — Core device lifecycle (initialize/shutdown/status)
- `PortDevice` — Port-based device communication
- `USBDevice` — USB device handling with libusb

### Communication

- `SerialPort` — Serial/COM port communication
- `USBPort` — USB HID device access
- `TCPPort` — Network-based device communication

### Device Types

- `FiscalRegister` — Fiscal receipt printing and reporting
- `CardReader` — Magnetic/smart card reading
- `CashAcceptor` — Bill validation and acceptance
- `CashDispenser` — Cash dispensing operations
- `Printer` — Receipt and barcode printing
- `Modem` — SMS and data communication

---

### Example: Fiscal Register Usage

```cpp
#include "Hardware/FR/FiscalRegister.h"
#include "Hardware/FR/Protocols/AtolProtocol.h"

FiscalRegister fr;
fr.setProtocol(new AtolProtocol());
fr.setPort("COM1");

if (fr.open()) {
    // Print receipt
    fr.printReceipt(receiptData);
    fr.close();
}
```

---

### Example: Card Reader Integration

```cpp
#include "Hardware/Cardreaders/CardReader.h"

CardReader reader;
reader.setPort("COM2");

connect(&reader, &CardReader::cardRead, this, &MyClass::onCardRead);

if (reader.initialize()) {
    reader.startReading();
}
```

---

### Example: Cash Acceptor

```cpp
#include "Hardware/CashAcceptors/CashAcceptor.h"

CashAcceptor acceptor;
acceptor.setPort("COM3");

connect(&acceptor, &CashAcceptor::billAccepted,
        this, &MyClass::onBillAccepted);

acceptor.enable();
```

---

## Integration

```cmake
target_link_libraries(MyApp PRIVATE
    HardwareCommon
    HardwareIOPorts
    HardwareFR
    HardwareCardreaders
    HardwareCashAcceptors
    # ... other hardware modules as needed
)
```

---

## Testing

- Unit tests: `tests/modules/Hardware/` — device simulation and protocol testing
- Integration tests: Hardware device connection and communication tests
- Run tests using `ctest -R Hardware` or project test targets

---

## Migration notes

- Adopted from TerminalClient project with EKiosk naming conventions
- CMake standardized to use `ek_add_library` with proper include paths
- Qt5/Qt6 compatibility maintained
- LibUSB integration moved to DEPENDS parameter

## Further reading

- Implementation & layout: `src/modules/Hardware/README.md` (internal structure and contributor notes)
- Device protocols: Individual protocol documentation in `src/modules/Hardware/Protocols/`
- See `tests/modules/Hardware/` for test examples and device simulation
