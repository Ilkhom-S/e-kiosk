# Hardware Module - Implementation Notes

See `docs/modules/hardware.md` for user-facing documentation, API usage, and integration guide.

## Implementation Structure

```
Hardware/
├── Common/          # Shared abstractions (DeviceBase, PortDevice, USBDevice)
├── IOPorts/         # Serial/USB/TCP port communication
├── Printers/        # Thermal receipt printing
├── Watchdogs/       # Hardware watchdog timers
├── FR/              # Fiscal registers (Atol, Shtrih, Prim, etc.)
├── Modems/          # AT command modems with SMS support
├── Cardreaders/     # Card readers (IDTech SDK integration)
├── CashAcceptors/   # Bill validators and acceptors
├── CashDispensers/  # Cash dispensing
├── CoinAcceptors/   # Coin validation
├── Scanners/        # Barcode scanners
└── Protocols/       # Device protocol implementations
```

## Build Dependencies

- Qt SerialPort module
- libusb (via thirdparty/LibUSB)
- SmsMessage (for modem SMS functionality)
- IDTech_SDK (for card readers)
- Log and SettingsManager modules

## CMake Notes

- Uses `ek_add_library` with standardized parameters
- INCLUDE_DIRS: `${CMAKE_SOURCE_DIR}/include` and `${CMAKE_CURRENT_SOURCE_DIR}`
- COMPILE_DEFINITIONS: `_UNICODE UNICODE` for Windows compatibility
- DEPENDS: Various thirdparty libraries (LibUSB, SmsMessage, IDTech_SDK)

## Platform-Specific Code

- Windows: Full hardware support with Setupapi/Advapi32 for FR
- Linux/macOS: Serial/USB ports supported; hardware availability limited
- LibUSB integration for cross-platform USB device access

## Testing

- Unit tests in `tests/modules/Hardware/`
- Mock devices for protocol testing
- Hardware integration tests require physical devices

## Migration Notes

- Ported from TerminalClient with EKiosk naming conventions
- CMake converted from `tc_add_library` to `ek_add_library`
- LibUSB linking moved to DEPENDS parameter
- TC_INCLUDES_DIR replaced with CMAKE_SOURCE_DIR/include
