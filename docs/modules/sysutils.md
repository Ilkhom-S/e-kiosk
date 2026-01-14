# SysUtils Module

The SysUtils module provides system utility functions for EKiosk, focusing on Windows-specific operations like printer management, process enumeration, privilege elevation, and cryptographic utilities.

## Purpose

SysUtils encapsulates low-level system interactions that are commonly needed across EKiosk applications. It provides a clean, platform-specific abstraction layer for operations that would otherwise require direct WinAPI calls, improving code maintainability and testability.

Key features:

- Printer status and configuration management
- Process enumeration and information retrieval
- Privilege elevation for system operations
- Cryptographic utilities (certificate validation, etc.)
- BOM removal from files
- Windows service and system control functions

## Implementation layout

For implementation details and file layout, see `src/modules/SysUtils/README.md` (implementation notes and contributor guidance).

## Dependencies

- **Qt Core**: For QString, QFile, QProcess, etc.
- **Windows APIs**: Winspool (printing), Wintrust (crypto), Advapi32 (privileges), Kernel32 (processes)
- **DelayImpHlp**: For dynamic import handling (vendored in thirdparty/)

## Usage

### Basic Usage

```cpp
#include <SysUtils/ISysUtils.h>

// Get the singleton instance
ISysUtils* sysUtils = ISysUtils::instance();

// Remove BOM from a file
sysUtils->rmBOM("path/to/file.txt");

// Get printer information
QVariantMap printerInfo = sysUtils->getPrinterInfo("PrinterName");

// Enumerate processes
QList<SProcessInfo> processes = sysUtils->getProcessesList();
```

### Printer Management

```cpp
// Get printer status
ulong status, attributes;
bool ok = getPrinterStatusData("PrinterName", jobs, status, attributes);

// Set printer to queued mode
QString error;
bool success = sysUtils->setPrintingQueuedMode("PrinterName", error);
```

### Privilege Elevation

```cpp
// Elevate privileges for shutdown
PrivilegeElevator privilege(SE_SHUTDOWN_NAME);
if (privilege.acquire()) {
    // Perform privileged operation
    privilege.release();
}
```

### Process Enumeration

```cpp
QList<SProcessInfo> processes = sysUtils->getProcessesList();
for (const auto& proc : processes) {
    qDebug() << "PID:" << proc.id << "Path:" << proc.path;
}
```

## CMake Integration

To use the SysUtils module in your application:

```cmake
# In your CMakeLists.txt
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
ek_add_application(MyApp
    SOURCES main.cpp
    QT_MODULES Core
    LIBRARIES SysUtils ek_common Advapi32 DelayImpHlp
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include
)
```

Note: SysUtils requires UNICODE builds on Windows. The module automatically defines UNICODE and \_UNICODE.

## Testing

Unit tests are located in `tests/unit/` and cover:

- File BOM removal
- Printer information retrieval
- Process enumeration
- Privilege elevation

Run tests with:

```bash
ctest -R SysUtils
```

## Platform Notes

- **Windows-only**: This module is currently Windows-specific due to its reliance on WinAPI calls.
- **Unicode builds**: Requires UNICODE and \_UNICODE definitions for proper wide character support.
- **Dependencies**: Links against Windows system libraries (Advapi32, Winspool, etc.) and the vendored DelayImpHlp library.

## Migration Notes

When migrating from direct WinAPI calls:

- Replace raw WinAPI functions with ISysUtils methods
- Handle QString conversions properly (SysUtils expects UTF-16 on Windows)
- Ensure UNICODE builds are enabled
- Include DelayImpHlp in your link dependencies
  <parameter name="filePath">c:\Projects\Humo\Kiosk\ekiosk\docs\sysutils-module.md
