# EKiosk Architecture Overview

EKiosk is a modular Qt C++ project for self-service kiosks. The architecture is split into clear layers and modules for maintainability and scalability.

## Documentation Index

- [Getting Started](getting-started.md)
- [Build Guide](build-guide.md)
- [Coding Standards](coding-standards.md)
- [Platform Compatibility](platform-compatibility.md)
- [Multilanguage Support](multilanguage.md)
- [Logging Module](logging.md)
- [BaseApplication Module](base_application.md)
- [SysUtils Module](sysutils-module.md)
- [Migration Plan & TODO](migration-todo.md)

## Key Concepts

- **Layered design:** UI, modules, devices, connection, utilities
- **Multiple executables:** All applications are in the `apps/` folder. Each app is a separate subfolder with its own code and build setup.
- **Per-app modular structure:**
  - Each app (e.g., `kiosk`, `Updater`, `UpdaterSplashScreen`, `WatchService`, `WatchServiceController`) has its own folder under `apps/`.
  - Each app contains its own `src/` (for code) and `CMakeLists.txt` (for build configuration).
  - Platform/build folders (e.g., `msvc/`) can be placed at the app root if needed.
- **Shared code:** To be refactored into top-level `src/` (shared code) and `include/` (public headers) as modularization proceeds.
- **Third-party libraries:** In `thirdparty/`.
- **Tests:** Mirror the structure of `src/` in the `tests/` folder for unit/integration tests.

### Applications in `apps/` (2026)

- `kiosk`: Main kiosk application
- `Updater`: Automatic software update manager
- `UpdaterSplashScreen`: Shows splash screen during update
- `WatchService`: (planned) Windows service for monitoring
- `WatchServiceController`: (planned) System tray controller for WatchService

### Modular CMake Setup

- The `apps/CMakeLists.txt` adds each app subdirectory using `add_subdirectory(<AppFolder>)`.
- Each app has its own `CMakeLists.txt` for building.
- The root `CMakeLists.txt` includes `add_subdirectory(apps)` to build all apps.

### Example (2026 Modular Structure)

```text
ekiosk/
├── apps/
│   ├── kiosk/
│   │   ├── src/
│   │   └── CMakeLists.txt
│   ├── Updater/
│   │   └── CMakeLists.txt (planned)
│   ├── UpdaterSplashScreen/
│   │   └── CMakeLists.txt (planned)
│   ├── WatchService/
│   │   └── CMakeLists.txt (planned)
│   ├── WatchServiceController/
│   │   └── CMakeLists.txt (planned)
│   └── ...
├── src/         # (future: shared code)
├── include/     # (future: shared headers)
├── thirdparty/
├── tests/
├── docs/
└── ...
```

See the above docs for details on each topic.

---

**For any new app or structural change, update this document and related docs to keep the architecture clear and up to date.**

---

_This file is now a summary. For details, see the linked documentation files._
