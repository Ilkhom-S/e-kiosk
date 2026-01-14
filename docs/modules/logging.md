# Logging Module

The EKiosk project uses a modular logging system based on the `ILog` interface, implemented by `SimpleLog` for file-based logging.

## Purpose

The logging module provides a lightweight, thread-safe logging solution for EKiosk applications. It replaces the previous `log4cpp` dependency with a simpler, custom implementation focused on file logging with daily rotation.

Key features:

- File-based logs with daily rotation (no size-based rotation)
- Severity levels: Debug, Info, Warning, Error
- Thread-safe operations
- Configurable log levels and destinations
- Memory-efficient compared to heavy-weight loggers

## Implementation layout

For implementation details and file layout, see `src/modules/common/log/README.md` (implementation notes and contributor guidance).

## Dependencies

- **Qt Core**: For QString, QFile, QDateTime, QMutex
- **SysUtils**: For rmBOM utility (removes BOM from log files)

## Usage

### Basic Usage

```cpp
#include <Common/ILog.h>

// Get or create a logger instance
ILog* logger = ILog::getInstance("MyLogger", LogType::File);
logger->setDestination("my_log");
logger->setLevel(LogLevel::Debug);

// Log messages
logger->write(LogLevel::Info, "Application started");
logger->write(LogLevel::Error, "An error occurred");
```

### Log Levels

- `LogLevel::Debug`: Detailed debug information
- `LogLevel::Normal`: General information (mapped to Info)
- `LogLevel::Warning`: Warning messages
- `LogLevel::Error`: Error messages

### Log File Location

Logs are written to `applicationDir/logs/YYYY.MM.DD <destination>.log`

Example: `C:/path/to/app/logs/2026.01.13 my_log.log`

## CMake Integration

To use the Log module in your application:

```cmake
# In your CMakeLists.txt
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
ek_add_application(MyApp
    SOURCES main.cpp
    QT_MODULES Core
    LIBRARIES Log SysUtils ek_common
)
```

## Testing

The module includes unit tests in `tests/unit/TestQFileLogger.cpp` that verify:

- Logger instance creation
- Message writing at different levels
- Log file creation and content

Run tests with:

```bash
ctest -R TestQFileLogger
```

## Migration Notes

This module replaces the previous `log4cpp` usage. During migration:

- Replace `ek::Log` calls with `ILog::getInstance()`
- Update log level constants
- Ensure SysUtils dependency is included for BOM removal
