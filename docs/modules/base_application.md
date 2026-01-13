# BaseApplication Module

## Overview

`BaseApplication` is a Qt application wrapper that provides common functionality for EKiosk executables, including single-instance enforcement, test mode detection, and basic logging setup.

## Purpose

BaseApplication serves as a foundation for EKiosk applications by handling:

- Single-instance enforcement using SingleApplication library
- Test mode detection (via command-line argument or environment variable)
- Basic application lifecycle management
- Integration point for logging and other common services

This module reduces boilerplate code in main functions and ensures consistent behavior across all EKiosk applications.

## Structure

```text
src/modules/common/application/
├── CMakeLists.txt              # Build configuration
├── src/
│   └── BaseApplication.cpp     # Implementation
└── include/
    └── Common/
        └── BasicApplication.h  # Public interface (note: header is BasicApplication.h)
```

## Dependencies

- **Qt Core**: For QApplication base class and QSettings
- **SingleApplication**: For single-instance enforcement (vendored in thirdparty/)
- **Log**: For file-based logging with ILog interface
- **Log**: For logging integration (optional, can be nullptr)

## Usage

### Basic Usage

```cpp
#include <Common/BasicApplication.h>

int main(int argc, char *argv[]) {
  BaseApplication app(argc, argv);

  // Check for single instance
  if (!app.isPrimaryInstance()) {
    return 1;  // Another instance is running
  }

  // Application logic here
  MainWindow window;
  window.show();

  return app.exec();
}
```

### Test Mode

BaseApplication automatically detects test mode:

- Command-line: `app.exe test`
- Environment: `EKIOSK_TEST_MODE=1`

In test mode, single-instance enforcement is disabled, allowing multiple instances for development and testing.

### Logging Integration

BaseApplication automatically initializes an ILog instance for file-based logging:

```cpp
// Logger is automatically created in constructor
ILog* logger = app.getLog(); // Returns the initialized logger
logger->setLevel(LogLevel::Debug); // Can adjust level as needed
```

The logger uses daily rotation and writes to `logs/YYYY.MM.DD/<app_name>.log`. Qt messages (qDebug, qWarning, etc.) are automatically routed through the logger.

## CMake Integration

To use BaseApplication in your application:

```cmake
# In your CMakeLists.txt
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
ek_add_application(MyApp
    SOURCES main.cpp
    QT_MODULES Core
    LIBRARIES BaseApplication SingleApplication ek_common
)
```

## Testing

The module includes basic tests for:

- Single-instance enforcement
- Test mode detection
- Application initialization

Run tests with:

```bash
ctest -R BaseApplication
```

## Migration Notes

When migrating from plain QApplication:

- Replace `QApplication` with `BaseApplication`
- Add single-instance check in main()
- Logging is automatically configured - use `getLog()` to access the logger
- Qt messages are automatically logged to files
- Ensure Log and SingleApplication libraries are linked
