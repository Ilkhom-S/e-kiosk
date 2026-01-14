# BaseApplication Module

## Purpose

BaseApplication is a Qt application wrapper that provides single-instance enforcement, test mode detection and basic logging initialization for EKiosk executables.

---

## Quick start 🔧

```cpp
#include <Common/BasicApplication.h>

int main(int argc, char *argv[]) {
    BaseApplication app(argc, argv);
    if (!app.isPrimaryInstance()) return 1;

    MainWindow w;
    w.show();
    return app.exec();
}
```

---

## Features

- Single-instance enforcement (via SingleApplication)
- Test mode detection (CLI or env var)
- Automatic logging initialization (ILog)

---

## Configuration

Enable test mode with `app.exe test` or `EKIOSK_TEST_MODE=1` environment variable for development scenarios.

---

## Usage / API highlights

- `BaseApplication app(argc, argv)` — initialize app with built-in behaviors
- `app.isPrimaryInstance()` — check for primary instance
- `app.getLog()` — access the configured logger

---

## Integration

Link against `BaseApplication`, `SingleApplication` and `Log` in your CMake target:

```cmake
target_link_libraries(MyApp PRIVATE BaseApplication SingleApplication Log)
```

---

## Testing

Tests include single-instance and test mode checks. Run with `ctest -R BaseApplication`.

---

## Further reading

- Implementation & layout: `src/modules/common/application/README.md` (implementation notes and contributor guidance)"}]}
