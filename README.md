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

### Environment Setup

For cross-platform development and to avoid CMake configuration issues, set Qt environment variables:

#### Windows

Run the setup script:

```cmd
setup-qt-env.bat
```

Or manually set environment variables:

```cmd
setx QT5_DIR "C:\Qt\5.15.2\msvc2019_64"
setx QT6_DIR "C:\Qt\6.10.1\msvc2022_64"
```

#### Linux/macOS

Run the setup script:

```bash
chmod +x setup-qt-env.sh
./setup-qt-env.sh
```

Or manually set environment variables:

```bash
export QT5_DIR="/opt/Qt/5.15.2/gcc_64"
export QT6_DIR="/opt/Qt/6.10.1/gcc_64"
```

### Using CMake (Recommended)

#### Option 1: Environment Variables (Cross-platform)

1. Set `QT5_DIR` or `QT6_DIR` environment variable (see above)
2. Configure: `cmake -S . -B build`
3. Build: `cmake --build build`

#### Option 2: CMake Presets

Available presets:

- `msvc`: MSVC build (Windows, recommended for Qt compatibility)
- `default`: Default build
- `mingw`: MinGW build (Windows)
- `release`: Release build

```bash
# Configure with MSVC preset (Windows)
cmake --preset msvc

# Or configure with default preset
cmake --preset default

# Build
cmake --build build-msvc  # or build (for default preset)
```

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
