# SysUtils

This folder contains the implementation of system utilities for all supported platforms (Windows, Linux, macOS).

- Canonical docs: `../../docs/modules/sysutils.md`

## Structure (implementation)

```text
src/modules/SysUtils/
├── CMakeLists.txt                    # Build configuration
├── src/
│   ├── windows/
│   │   ├── CryptSysUtils.cpp         # Windows: Cryptographic utilities
│   │   ├── PrinterSysUtils.cpp       # Windows: Printer management
│   │   ├── PrivilegeElevator.cpp     # Windows: Privilege elevation
│   │   └── SysUtils.cpp              # Windows: General system utilities
│   └── unix/
│       ├── CryptSysUtils.cpp         # Linux/macOS: Cryptographic utilities
│       ├── PrinterSysUtils.cpp       # Linux/macOS: Printer management
│       └── SysUtils.cpp              # Linux/macOS: General system utilities
└── include/
    └── SysUtils/
        ├── ISysUtils.h               # Public interface
        ├── windows/                  # Windows platform headers
        │   ├── CryptSysUtils.h
        │   ├── PrinterSysUtils.h
        │   ├── PrivilegeElevator.h
        │   └── SysUtils.h
        └── unix/                     # Linux/macOS platform headers
            ├── CryptSysUtils.h
            ├── PrinterSysUtils.h
            └── SysUtils.h
```

**Contributor notes:**

- Keep high-level usage and examples in `docs/modules/sysutils.md`.
- Use this README for platform notes, build hints, and implementation specifics.
- The module is fully cross-platform; see CMakeLists.txt for platform-specific build logic.
