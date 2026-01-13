# BaseApplication

## Overview

`BaseApplication` is a lightweight application wrapper intended to provide common behavior for EKiosk executables:

- Single-instance enforcement
- Test mode detection (via `test` CLI argument or `EKIOSK_TEST_MODE` env var)
- Simple file-based logging
- Helper to start detached processes

## Usage

In your `main()` replace `QApplication` with `BaseApplication`:

```cpp
#include <Application/BaseApplication.h>

int main(int argc, char *argv[]) {
  BaseApplication app(argc, argv);
  if (!app.isPrimaryInstance()) return 1;
  // ...
  return app.exec();
}
```

The module lives under `src/modules/common/application` and the public header is `include/Common/BasicApplication.h`.

- The old `BaseApplication` concrete wrapper (previously `include/Common/BaseApplication.h`) has been removed; prefer using `BasicApplication` together with a local `QApplication` instance in `main()` as shown in the example below.
- Note: `BasicApplication::getLog()` returns nullptr. An `ILog` implementation and logging adapter should be ported later as a separate module or dependency and wired in at the application level.
