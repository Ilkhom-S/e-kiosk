# DebugUtils Module

## Purpose

Provides debugging utilities for call stack dumping, unhandled exception handling and trace logging to help diagnose crashes and runtime issues.

---

## Quick start 🔧

```cpp
#include <DebugUtils/DebugUtils.h>

QStringList stack;
DumpCallstack(stack, nullptr);
// use stack for diagnostics
```

---

## Features

- Call stack dumping
- Unhandled exception handler installation
- Trace logging macros for function entry/exit

---

## Platform support

| Platform | Status           | Notes                            |
| -------- | ---------------- | -------------------------------- |
| Windows  | ✅ Full          | Uses dbghelp and Windows APIs    |
| Linux    | ❌ Not supported | Windows-specific stack APIs used |
| macOS    | ❌ Not supported | Windows-specific stack APIs used |

---

## Configuration

No runtime configuration; use the provided APIs to enable or configure trace logging and exception handlers.

---

## Usage / API highlights

- `DumpCallstack(QStringList &out, QObject *context)` — capture current call stack
- `SetUnhandledExceptionsHandler(handler)` — install global unhandled exception handler
- `ENABLE_TRACE_LOGGER("Module")` / `LOG_TRACE()` — trace logging helpers

---

## Integration

Link the module to your target:

```cmake
target_link_libraries(MyApp PRIVATE DebugUtils)
```

---

## Testing

Unit tests live in `tests/modules/DebugUtils/` and can be run through the project test target.

---

## Further reading

- Implementation & layout: `src/modules/DebugUtils/README.md` (internal details and contributor notes)"
