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

- **Build**: Use qmake to generate Makefile from `ekiosk.pro`, then `make` (or `mingw32-make` on Windows). VS Code task: `dotnet build` (configured in `tasks.json` for Qt compatibility).
- **Run**: Execute `../EKiosk/EKiosk.exe` (TARGET in .pro sets output path).
- **Debug**: Attach debugger to running process; use Qt Creator for GUI debugging. Check logs in `src/other/logger.h` for issues.
- **Dependencies**: Requires Qt 5+ (widgets, webkit, serialport, etc.); Windows-specific libs (rasapi32, winspool).

## Integration Points

- **External APIs**: JSON-based requests in `src/modules/JsonRequest.cpp` for web services.
- **Hardware**: Serial communication via `QSerialPort` in device classes (e.g., `CCTalk.cpp`).
- **Database**: SQL queries in `src/db/sqlconnection.h` (likely SQLite or similar).
- **Network**: WebSockets in `src/mainwindow.cpp` for real-time updates.

## Common Pitfalls

- Ensure `QT += widgets` in .pro for Qt 5 compatibility.
- Device classes inherit from abstract bases; implement virtual methods.
- Use `QSharedMemory` and `QSystemSemaphore` for single-instance checking (in `main.cpp`).

Reference: `ekiosk.pro` for build config, `src/mainwindow.cpp` for app entry, `src/devices/` for hardware patterns.
