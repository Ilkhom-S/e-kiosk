# Logging Module

## Purpose

The Logging module provides a lightweight, thread-safe logging solution for EKiosk applications via the `ILog` interface. It supports file-based logs with daily rotation and configurable log levels.

---

## Quick start 🔧

```cpp
#include <Common/ILog.h>

auto logger = ILog::getInstance("MyLogger", LogType::File);
logger->setDestination("my_log");
logger->setLevel(LogLevel::Debug);
logger->write(LogLevel::Info, "Application started");
```

---

## Features

- File-based logs with daily rotation
- Severity levels: Debug, Info (Normal), Warning, Error
- Thread-safe operations
- Configurable destinations and log levels

---

## Platform support

| Platform | Status  | Notes                        |
| -------- | ------- | ---------------------------- |
| Windows  | ✅ Full | File-based logging supported |
| Linux    | ✅ Full | File-based logging supported |
| macOS    | ✅ Full | File-based logging supported |

---

## Configuration

No global config file; configure loggers at runtime with `ILog` API. Log files are written to:

```text
<applicationDir>/logs/YYYY.MM.DD <destination>.log
```

Previous days' logs are archived as `YYYY.MM.DD_logs.7z` in the same directory.

---

## Log File Format

### Location

Log files are stored in the `logs/` subdirectory relative to the application executable directory.

### Naming Convention

Files are named according to the pattern: `YYYY.MM.DD <destination>.log`

Where:

- `YYYY.MM.DD` - current date
- `<destination>` - subsystem or component name (set via `setDestination()`)

### Archive Format

Logs from previous days are automatically archived into 7zip files named `YYYY.MM.DD_logs.7z` in the same logs directory.

### Message Format

Each log entry follows the format:

```text
hh.mm.ss.zzz [L] Message
```

Where:

- `hh.mm.ss.zzz` - timestamp (hours.minutes.seconds.milliseconds)
- `[L]` - log level indicator
- `Message` - the actual log message

### Log Levels

- `D` - Debug message (developer-only information)
- `I` - Info message (normal operational information)
- `W` - Warning (potential issues that don't prevent operation)
- `E` - Error (issues that may affect functionality)
- `C` - Critical error (severe issues that may prevent operation)

### Standard Log Names

Common subsystem log names used in EKiosk:

- `EKiosk` - Main application log. Marks application start/stop, configuration loading, and module initialization
- `Ad` - Advertising subsystem
- `BillAcceptor on COMx` - Bill acceptor device
- `CoinAcceptor on COMx` - Coin acceptor device
- `Connection` - Network connectivity monitoring
- `FirmwareUpload` - Device firmware update scenarios
- `Funds` - Cash and electronic funds tracking
- `Health` - System health monitoring
- `HID` - Human Interface Device subsystem (scanner, camera, card reader)
- `Interface` - User interface display subsystem
- `MessageQueueClient` - Inter-component message exchange
- `Monitoring` - Terminal monitoring client
- `Payments` - Payment processing system
- `PinLoader` - Scratch card denomination loader
- `Printer` - System printer
- `Printer on COMx` - Serial printer
- `POSPrinter on COMx` - POS printer
- `QtMessages` - Qt framework system messages
- `ReportBuilder` - Update system reports
- `Scanner` - Barcode scanner
- `Scheduler` - Internal task scheduler
- `Updater/UpdaterTrace` - Update system
- `UserAssistant` - User assistance module
- `WatchService` - Service management module
- `WatchServiceController` - Tray application log
- `Watchdog` - Watchdog device driver

### Application Lifecycle Markers

- `Scheduler` - Internal task scheduler
- `Updater/UpdaterTrace` - Update system
- `UserAssistant` - User assistance module
- `WatchService` - Service management module
- `WatchServiceController` - Tray application log
- `Watchdog` - Watchdog device driver

### Application Lifecycle Markers

Each application start and stop is marked in the main log with entries like:

```text
02:45:14.670 [I] ******************************** LOG [EKiosk] STOP.
02:45:18.585 [I] ******************************** LOG [EKiosk] STARTED. EKiosk 3.0.0 build 202601131240.
```

Missing stop markers indicate abnormal termination. Possible causes:

- Forced termination via Task Manager
- Power loss (all logs terminate simultaneously)
- Forced shutdown due to unresponsive main thread (see WatchService.log)
- Application crash due to error

---

## Usage / API highlights

- `ILog::getInstance(name, type)` — obtain or create a logger
- `logger->setDestination()` — configure file destination
- `logger->setLevel()` — set the runtime level

---

## Integration

Add `Log` (and `SysUtils` if needed) to your CMake target:

```cmake
target_link_libraries(MyApp PRIVATE Log SysUtils)
```

---

## Testing

Unit tests live in `tests/unit/` (e.g., `TestQFileLogger.cpp`). Run tests with `ctest -R TestQFileLogger`.

---

## Migration notes

This module replaces `log4cpp`. Migration steps:

- Replace previous logging calls with `ILog::getInstance()` and the corresponding APIs.

---

## Further reading

- Implementation & file layout: `src/modules/common/log/README.md` (internal APIs and contributor notes)
