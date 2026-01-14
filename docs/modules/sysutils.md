# SysUtils Module

## Purpose

SysUtils encapsulates Windows-specific system utilities such as printer management, privilege elevation, process enumeration and cryptographic helpers.

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
- Privilege elevation helpers
- Cryptographic and certificate utilities
- BOM removal helper utilities

---

## Platform support

| Platform | Status           | Notes                                           |
| -------- | ---------------- | ----------------------------------------------- |
| Windows  | ✅ Full          | Relies on WinAPI (Advapi32, Wintrust, Winspool) |
| Linux    | ❌ Not supported | Windows-specific APIs used                      |
| macOS    | ❌ Not supported | Windows-specific APIs used                      |

---

## Configuration

No global configuration; APIs are used directly. Note: SysUtils expects UNICODE builds on Windows.

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
target_link_libraries(MyApp PRIVATE SysUtils Advapi32 Winspool)
```

---

## Testing

Unit tests covering SysUtils live in `tests/unit/` and can be run with `ctest -R SysUtils`.

---

## Further reading

- Implementation & layout: `src/modules/SysUtils/README.md` (platform notes and contributor guidance)"
