# Hardware Modules

Hardware abstraction and device communication modules.

## Structure

```
Hardware/
├── Common/          # Shared abstractions
├── IOPorts/         # Serial/USB ports
├── Printers/        # Printer support
├── Watchdogs/       # Watchdog timers
├── FR/              # Fiscal registers
├── Modems/          # Modem support
├── Cardreaders/     # Card readers
├── CashAcceptors/   # Bill validators
└── CashDispensers/  # Cash dispensers
```

## Modules

### HardwareCommon

Base classes for all hardware:

- `DeviceBase` - Common device functionality
- `PortDevice` - Port-based device base
- `USBDevice` - USB device base

### HardwareIOPorts

Port communication:

- COM/Serial ports
- USB HID
- TCP/IP ports
- libusb integration

### HardwarePrinters

Printer abstractions:

- Thermal receipt printing
- Barcode/QR generation
- Paper status

### Other Modules

See individual module documentation.

## Usage

```cpp
#include "Hardware/Common/DeviceBase.h"
#include "Hardware/IOPorts/SerialPort.h"

class MyDevice : public DeviceBase {
    SerialPort* m_port;

    bool initialize() override {
        m_port = new SerialPort("COM1", 9600);
        return m_port->open();
    }
};
```

## Platform Support

| Module        | Windows | Linux | macOS |
| ------------- | ------- | ----- | ----- |
| Common        | ✅      | ✅    | ✅    |
| IOPorts       | ✅      | 🔬    | 🔬    |
| Printers      | ✅      | ❌    | ❌    |
| Watchdogs     | ✅      | ❌    | ❌    |
| FR            | ✅      | ❌    | ❌    |
| Cardreaders   | ✅      | 🔬    | 🔬    |
| CashAcceptors | ✅      | 🔬    | 🔬    |

## Dependencies

- Qt SerialPort module
- libusb (thirdparty)
- `Log` module
- `SettingsManager` module
