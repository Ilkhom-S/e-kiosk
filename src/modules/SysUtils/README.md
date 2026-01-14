# SysUtils

This folder contains the implementation of system utilities (Windows-specific).

- Canonical docs: `../../docs/modules/sysutils.md`

## Structure (implementation)

```text
src/modules/SysUtils/
├── CMakeLists.txt                    # Build configuration
├── src/
│   └── windows/
│       ├── CryptSysUtils.cpp         # Cryptographic utilities
│       ├── PrinterSysUtils.cpp       # Printer management
│       ├── PrivilegeElevator.cpp     # Privilege elevation
│       └── SysUtils.cpp              # General system utilities
└── include/
    └── SysUtils/
        ├── ISysUtils.h               # Public interface
        └── windows/                  # Platform headers
            ├── CryptSysUtils.h
            ├── PrinterSysUtils.h
            ├── PrivilegeElevator.h
            └── SysUtils.h
```

**Contributor notes:**

- Keep high-level usage and examples in `docs/modules/sysutils.md`.
- Use this README for platform notes, build hints, and implementation specifics.
