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

## Module Organization

Each module documentation includes:

- **Purpose**: Why the module exists and what problems it solves
- **Structure**: Folder layout and file organization
- **Dependencies**: Required libraries and frameworks
- **Usage**: Code examples and integration patterns
- **CMake Integration**: How to link and build with the module
- **Testing**: Unit test information
- **Migration Notes**: Guidance for adopting the module

## Adding New Modules

When adding a new module:

1. Create documentation following the established pattern
2. Place the .md file in this directory
3. Update this README.md with the new module entry
4. Update the main [architecture documentation](../architecture.md) index

## Dependencies Between Modules

- **Log** depends on **SysUtils** (for BOM removal)
- **BaseApplication** depends on **Log** (for file-based logging)
- **SysUtils** is standalone but used by other modules

See individual module documentation for detailed dependency information.
