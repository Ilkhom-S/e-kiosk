# EKiosk Modules Documentation

This directory contains documentation for all EKiosk modules. Each module provides specific functionality and can be used independently across applications.

## Available Modules

### [Logging Module](logging.md)

Provides file-based logging with daily rotation, replacing the previous log4cpp dependency.

- **Purpose**: Lightweight, thread-safe logging
- **Key Features**: Daily log rotation, configurable levels, memory-efficient
- **Dependencies**: Qt Core, SysUtils

### [BaseApplication Module](base_application.md)

Qt application wrapper providing common functionality for EKiosk executables.

- **Purpose**: Single-instance enforcement, test mode detection, application lifecycle
- **Key Features**: Single-instance apps, test mode support, logging integration point
- **Dependencies**: Qt Core, SingleApplication

### [SysUtils Module](sysutils.md)

Windows system utilities abstraction layer for low-level operations.

- **Purpose**: Clean interface for Windows-specific system operations
- **Key Features**: Printer management, process enumeration, privilege elevation
- **Dependencies**: Qt Core, Windows APIs, DelayImpHlp
- **Platform**: Windows-only

### [DebugUtils Module](debugutils.md)

Debugging utilities for call stack analysis and exception handling.

- **Purpose**: Runtime debugging and crash analysis tools
- **Key Features**: Call stack dumping, unhandled exception handling, trace logging
- **Dependencies**: Qt Core, Windows APIs, StackWalker library
- **Platform**: Windows-only

### [Hardware Module](hardware.md)

Device abstraction and communication layers for kiosk hardware peripherals.

- **Purpose**: Unified APIs for fiscal registers, card readers, cash devices, printers, and modems
- **Key Features**: Device lifecycle management, protocol abstraction, cross-platform hardware support
- **Dependencies**: Qt Core, Qt SerialPort, libusb, SmsMessage, IDTech_SDK
- **Platform**: Cross-platform with Windows focus

### [Connection Module](connection.md)

Network connectivity management and monitoring.

- **Purpose**: Unified interface for various connection types
- **Key Features**: Dial-up, local network, remote connections, automatic monitoring
- **Dependencies**: Qt Core, Windows RAS APIs, NetworkTaskManager
- **Platform**: Windows-only

### [NetworkTaskManager Module](networktaskmanager.md)

Provides HTTP/HTTPS network operations and task management.

- **Purpose**: Centralized network requests, retries, and SSL handling
- **Key Features**: Request queue and retries, connection pooling, SSL/TLS configuration, timeout handling, request/response logging
- **Dependencies**: Qt Core, Qt Network, Log, SettingsManager
- **Platform**: Cross-platform (Windows, Linux, macOS)

### [Watchdog Service](watchdog.md)

Application monitoring and management service with automatic restart capabilities.

- **Purpose**: Monitor application modules, manage startup/shutdown priorities, block forbidden applications
- **Key Features**: Module lifecycle management, priority-based startup/shutdown, forbidden application blocking, cross-platform process monitoring
- **Dependencies**: Qt Core, Qt Widgets, SysUtils, DebugUtils, SettingsManager, MessageQueue
- **Platform**: Cross-platform (Windows, Linux, macOS)

## Module Organization

Each module documentation includes:

- **Purpose**: Why the module exists and what problems it solves
- **Structure**: Folder layout and file organization (kept in `src/modules/<module>/README.md`)
- **Dependencies**: Required libraries and frameworks
- **Platform support**: Supported platforms and notes (Windows, Linux, macOS)
- **Usage**: Code examples and integration patterns
- **CMake Integration**: How to link and build with the module
- **Testing**: Unit and module test locations and commands (see the [Testing Guide](../testing.md) for details)
- **Migration Notes**: Guidance for adopting the module

## Adding New Modules

When adding a new module:

1. Create documentation following the established pattern
2. Place the .md file in this directory
3. Update this README.md with the new module entry
4. Update the main [architecture documentation](../architecture.md) index

**Testing checklist for new modules:**

- Add tests under `tests/modules/<Module>/` or `tests/unit/` as appropriate.
- Add the test to CMake using `ek_add_test()` so tests run in CI and via `ctest`.
- Add a **Testing** section to `docs/modules/<module>.md` that lists the test files and commands to run them locally.

## Dependencies Between Modules

- **Log** depends on **SysUtils** (for BOM removal)
- **BaseApplication** depends on **Log** (for file-based logging)
- **SysUtils** is standalone but used by other modules

See individual module documentation for detailed dependency information.
