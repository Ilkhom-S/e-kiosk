# DebugUtils Module

## Purpose

The DebugUtils module provides debugging utilities for EKiosk applications, including call stack dumping and unhandled exception handling. It helps with diagnosing crashes and runtime issues in production environments.

## Implementation layout

For implementation details and file layout, see `src/modules/DebugUtils/README.md` (implementation notes and contributor guidance).

## Dependencies

- **Qt Core**: For QString and other utilities
- **Windows APIs**: dbghelp.dll, kernel32.dll for stack walking and exception handling
- **Third-party**: Modified StackWalker library for cross-platform stack tracing

## Platform Compatibility

- **Windows**: Fully supported (MSVC, MinGW)
- **Linux/macOS**: Not supported (Windows-specific APIs)

## Usage

### Basic Call Stack Dumping

```cpp
#include <DebugUtils/DebugUtils.h>

void someFunction() {
    QStringList stack;
    DumpCallstack(stack, nullptr);  // Dump current call stack
    // stack now contains function names and line numbers
}
```

### Unhandled Exception Handling

```cpp
#include <DebugUtils/DebugUtils.h>

int main(int argc, char *argv[]) {
    SetUnhandledExceptionsHandler(myExceptionHandler);
    // Application code...
}
```

### Trace Logging

```cpp
#include <DebugUtils/TraceLogger.h>

ENABLE_TRACE_LOGGER("MyModule");

void myFunction() {
    LOG_TRACE();  // Logs function entry/exit with indentation
}
```

## CMake Integration

Add to your application's CMakeLists.txt:

```
cmake
target_link_libraries(MyApp PRIVATE DebugUtils)
```

## Testing

Unit tests are located in `tests/modules/DebugUtils/`. Run with:

```bash
cmake --build build --target test
```

## Migration Notes

Replace manual stack walking code with this module for consistent error reporting across the application.
