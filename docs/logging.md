# Logging

This project includes a lightweight QFile-based logger adapted from the
TerminalClient project's logging module. The implementation is intended to be
memory-efficient and simple compared to log4cpp and similar heavy-weight
loggers.

Key features:

- File-based logs with rotation by size
- Simple severity levels: Debug, Info, Warning, Error
- Thread-safe (mutex-protected writes)

Usage:

- Initialise early in your application startup (for example in `main`):

```cpp
#include <Common/Log.h>

int main(int argc, char** argv) {
  QApplication app(argc, argv);
  ek::Log::init(QDir::tempPath() + "/ekiosk.log", ek::LogLevel::Info, 10*1024*1024, 5);
  ek::Log::info("Application started");
  ...
}
```

Migration plan:

- The logger is added to the repository and built as a library (`Log`), but
  it is _not_ yet replacing the existing `log4cpp` usage. This allows a
  gradual migration: update modules to use the new logger during refactors
  and remove the `log4cpp` dependency later.

Tests:

- A unit test `TestQFileLogger` verifies log file creation and rotation.

License:

- The logger code in this repository is an independent implementation inspired
  by TerminalClient's logfile-based approach. If you want exact code copied
  from TerminalClient, provide the files or confirm permission and I will copy
  them verbatim including license headers.
