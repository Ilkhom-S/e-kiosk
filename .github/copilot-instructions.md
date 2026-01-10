# EKiosk Qt C++ Project - AI Coding Agent Instructions

## Architecture Overview

This is a Qt-based kiosk application (EKiosk) for payment processing and device management. The architecture follows a modular design with clear separation between UI, business logic, device interfaces, and network communication.

- **UI Layer**: `src/mainwindow.cpp` and `src/ui/` contain Qt widgets and dialogs for user interaction.
- **Modules Layer**: `src/modules/` handles business logic like authentication (`AuthRequest.cpp`), balance checks (`GetBalanceAgent.cpp`), and OTP handling (`SendOtp.cpp`).
- **Devices Layer**: `src/devices/` manages hardware interfaces (coin acceptors in `coinacceptor/`, printers in `printers/`, validators in `validators/`, modems in `modems/`, watchdogs in `watchdogs/`).
- **Connection Layer**: `src/connection/` handles network (`Connect.cpp`) and RAS dial-up (`RasConnection.cpp`).
- **Utilities**: `src/other/` for logging (`logger.h`), receipts (`receipt.cpp`), and system info (`systemInfo.h`).

Data flows from UI → modules for service requests → devices/network for execution, with results fed back via signals/slots.

## Key Patterns and Conventions

- **Signals/Slots**: Use old macro syntax (`SIGNAL(timeout())`, `SLOT(handleTimeout())`) for Qt connections, as seen in `src/updater/update.cpp` (e.g., `connect(timer, SIGNAL(timeout()), this, SLOT(update()))`).
- **File Structure**: Each class has `.cpp`/`.h` pair; UI files have `.ui` for Qt Designer.
- **Error Handling**: Log errors via `logger.h` functions; check return values from device operations.
- **Device Abstraction**: Abstract base classes (e.g., `AbstractPrinter.cpp`) with concrete implementations (e.g., `CitizenCBM1000.cpp`).
- **Configuration**: Constants in `src/main.h` (e.g., `wsPort = 9090`); no external config files.

## Build and Run Workflow

- **Build**: Use CMake with Qt Kits for cross-platform builds. Configure Qt installations via VS Code Qt Extension or cmake-kits.json. VS Code automatically detects Qt versions and switches IntelliSense paths.
- **Run**: Execute `EKiosk/Debug/EKiosk.exe` (from MSVC build) or `../EKiosk/EKiosk.exe` (from qmake build).
- **Debug**: Attach debugger to running process; use Qt Creator for GUI debugging. Check logs in `src/other/logger.h` for issues.
- **Dependencies**: Requires Qt 5+ (widgets, serialport, websockets, etc.); Windows-specific libs (rasapi32, winspool). Configure Qt installations via VS Code Qt Extension or cmake-kits.json for automatic path detection.

## Integration Points

- **External APIs**: JSON-based requests in `src/modules/JsonRequest.cpp` for web services.
- **Hardware**: Serial communication via `QSerialPort` in device classes (e.g., `CCTalk.cpp`).
- **Database**: SQL queries in `src/db/sqlconnection.h` (likely SQLite or similar).
- **Network**: WebSockets in `src/mainwindow.cpp` for real-time updates.

## Common Pitfalls

- Ensure `QT += widgets` in .pro for Qt 5 compatibility.
- Device classes inherit from abstract bases; implement virtual methods.
- Use `QSharedMemory` and `QSystemSemaphore` for single-instance checking (in `main.cpp`).
- **Qt Compiler Compatibility**: On Windows, use MSVC preset (`cmake --preset msvc`) when Qt is compiled with MSVC. MinGW builds will fail with undefined `__imp__` symbols if Qt uses MSVC libraries.
- **Qt Kit Management**: Use VS Code Qt Extension to register Qt installations as kits, or manually configure cmake-kits.json. Switch kits via status bar for automatic IntelliSense path updates.
- **Environment Variables**: No longer needed - Qt paths are managed through kits instead of QT5_DIR/QT6_DIR variables.
- **Web Components**: The project does not use Qt WebEngine/WebView components despite CMake configuration - they have been removed from the build to avoid unnecessary dependencies.

## Documentation and Commits

- **Reflect Changes in Docs**: Any architectural changes, new features, or significant updates must be documented in `docs/` (e.g., update `docs/architecture.md` for structural changes).
- **Conventional Commits**: Use conventional commit format for all commits (e.g., `feat: add new module`, `fix: resolve device connection issue`). Follow [Conventional Commits](https://conventionalcommits.org/) standard.

## External References

- **Reference Project**: [TerminalClient](https://github.com/Ilkhom-S/TerminalClient) - A complete Qt5 C++ kiosk application. Use as reference for architecture, structure, codebase, and device implementations. Aim to align EKiosk with similar patterns or copy ready device codes.
- **Migration Source**: [TCPKiosk](https://github.com/PVG-Kiosk/TCPKiosk) - A .NET 4 C# kiosk project. Migrate features and functionality to this C++ Qt 5/6 project.

Reference: `ekiosk.pro` for build config, `src/mainwindow.cpp` for app entry, `src/devices/` for hardware patterns.
