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
