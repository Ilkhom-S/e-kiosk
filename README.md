# EKiosk

EKiosk is a Qt-based C++ application for payment kiosk systems, handling device management, user authentication, and transaction processing.

## Features

- Payment processing with various devices (coin acceptors, printers, validators)
- User authentication and OTP handling
- Network connectivity and web service integration
- Modular architecture for hardware abstraction

## Requirements

- Qt 5.0 or later
- C++11 compatible compiler
- **Platforms**: Windows (minimum Windows 7 by end of 2026, currently Windows XP with caveats), Linux (primary), macOS (alternative)

## Building

### Using CMake (Recommended)

1. Ensure Qt 5 and CMake are installed.
2. Create build directory: `mkdir build && cd build`
3. Configure: `cmake ..`
4. Build: `cmake --build .`
5. Executable will be in `../EKiosk/EKiosk.exe`.

### Using QMake (Legacy)

1. Ensure Qt 5 is installed and qmake is in PATH.
2. Run `qmake ekiosk.pro` to generate Makefile.
3. Run `make` (or `mingw32-make` on Windows) to build.
4. Executable will be in `../EKiosk/EKiosk.exe`.

## Running

Execute `../EKiosk/EKiosk.exe`. Use `--test` for test mode.

## Project Structure

- `src/`: Source code
  - `main.cpp`: Application entry point
  - `mainwindow.cpp`: Main UI window
  - `ui/`: UI dialogs and forms
  - `modules/`: Business logic modules
  - `devices/`: Hardware device interfaces
  - `connection/`: Network connections
  - `other/`: Utilities and helpers
- `assets/`: Images, sounds, styles
- `ekiosk.pro`: QMake project file

## Contributing

See `docs/` for architecture and migration guides.
