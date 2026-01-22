# SysUtils Module

## Purpose

SysUtils encapsulates cross-platform system utilities such as printer management, privilege elevation, process enumeration, and cryptographic helpers for Windows, Linux, and macOS.

---

## Quick start 🔧

```cpp
#include <SysUtils/ISysUtils.h>

auto sysUtils = ISysUtils::instance();
sysUtils->rmBOM("path/to/file.txt");
QVariantMap info = sysUtils->getPrinterInfo("PrinterName");
```

---

## Features

- Printer status and management
- Process enumeration
- Privilege elevation helpers (where supported)
- Cryptographic and certificate utilities
- BOM removal helper utilities

---

## Platform support

| Platform | Status  | Notes                                                              |
| -------- | ------- | ------------------------------------------------------------------ |
| Windows  | ✅ Full | Relies on WinAPI (Advapi32, Wintrust, Winspool)                    |
| Linux    | ✅ Full | Uses POSIX, CUPS, OpenSSL, and standard Linux APIs                 |
| macOS    | ✅ Full | Uses POSIX, CUPS, OpenSSL, and macOS-specific APIs where necessary |

---

## Configuration

No global configuration; APIs are used directly. On Windows, SysUtils expects UNICODE builds. On Linux/macOS, ensure CUPS and OpenSSL are available for full feature support.

---

## Usage / API highlights

- `ISysUtils::instance()` — singleton access
- `rmBOM(path)` — remove BOM from files
- `getPrinterInfo(name)` — retrieve printer status
- `getProcessesList()` — enumerate processes

---

## Integration

Link the module and required system libraries in CMake:

```cmake
# Windows
if(WIN32)
    target_link_libraries(MyApp PRIVATE SysUtils Advapi32 Winspool)
else()
    target_link_libraries(MyApp PRIVATE SysUtils)
endif()
```

---

## Testing

Unit tests covering SysUtils live in `tests/unit/` and can be run with `ctest -R SysUtils`.

---

## Further reading

- Implementation & layout: `src/modules/SysUtils/README.md` (platform notes and contributor guidance)
