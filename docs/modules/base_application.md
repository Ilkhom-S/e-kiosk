# BasicApplication Module

## Purpose

BasicApplication is a Qt application wrapper that provides configuration loading from INI files, single-instance enforcement, test mode detection, logging initialization, and cross-platform OS utilities for EKiosk executables. It serves as the base class for all EKiosk applications, ensuring consistent behavior and settings management.

---

## Quick start 🔧

```cpp
#include <Common/BasicApplication.h>

int main(int argc, char *argv[]) {
    BasicApplication app("MyApp", "1.0.0", argc, argv);

    // App is ready with settings loaded, logging initialized
    MainWindow w;
    w.show();
    return app.exec(); // If using Qt, wrap in BasicQtApplication
}
```

For Qt GUI apps with translation support:

```cpp
#include <Common/BasicApplication.h>

int main(int argc, char *argv[]) {
    BasicQtApplication<QApplication> app("MyApp", "1.0.0", argc, argv);

    MainWindow w;
    w.show();
    return app.exec();
}
```

---

## Features

- **Configuration Loading**: Automatically loads settings from `appname.ini` and user-specific `user.ini`
- **Working Directory Management**: Configurable via INI for flexible deployment
- **Logging**: Integrated ILog with configurable levels from user settings
- **Single-instance Enforcement**: Automatic via SingleApplication library (use `BasicQtApplication<SingleApplication>` for explicit control)
- **Test Mode Detection**: CLI argument or environment variable for development
- **OS Utilities**: Cross-platform OS version detection
- **Qt Integration**: BasicQtApplication template for Qt apps with translation loading

---

## Platform support

| Platform | Status  | Notes                                        |
| -------- | ------- | -------------------------------------------- |
| Windows  | ✅ Full | Full support with COM initialization for WMI |
| Linux    | ✅ Full | Supported via Qt and SingleApplication       |
| macOS    | ✅ Full | Supported via Qt and SingleApplication       |

---

## Configuration

BasicApplication loads configuration from two INI files:

1. **Main Configuration** (`appname.ini`): Located in the same directory as the executable. Contains application-wide settings.
2. **User Configuration** (`user.ini`): Located in the user data directory (specified by `common/user_data_path` in main config). Contains user-specific overrides.

### Main Configuration Keys (`appname.ini`)

- `common/working_directory`: Overrides the default working directory (executable's directory). Can be absolute or relative path.
- `common/user_data_path`: Directory for user-specific data and `user.ini` (relative to working directory).

### User Configuration Keys (`user.ini`)

- `log/level`: Global log level (integer). Valid range: 0 (Off) to 5 (Max). Maps to LogLevel enum.

### Example `appname.ini`

```ini
[common]
working_directory=../data
user_data_path=userdata
```

### Example `user.ini`

```ini
[log]
level=3
```

### Test Mode

Enable test mode (disables kiosk restrictions) via:

- Command line: `app.exe test`
- Environment variables: `EKIOSK_TEST_MODE=1` or `TEST_MODE=1`

---

## Usage / API highlights

- `BasicApplication(name, version, argc, argv)` — Initialize with app metadata and arguments
- `app.getSettings()` — Access loaded QSettings (read-only for app config)
- `app.getWorkingDirectory()` — Get configured working directory
- `app.getLog()` — Access the initialized logger
- `app.isTestMode()` — Check if running in test mode
- `BasicQtApplication<T>(name, version, argc, argv)` — For Qt apps (T = QApplication, QCoreApplication, etc.)
- `qtApp.getQtApplication()` — Access the underlying Qt app instance

---

## Integration

Link against required modules in your CMake target:

```cmake
target_link_libraries(MyApp PRIVATE BasicApplication SingleApplication Log DebugUtils SysUtils)
```

Use the EKiosk CMake helpers:

```cmake
ek_add_application(MyApp ...)
target_link_libraries(MyApp PRIVATE BasicApplication ...)
```

---

## Testing

Unit tests cover configuration loading, test mode detection, and single-instance behavior. Run with:

```bash
ctest -R BasicApplication
```

---

## Migration notes

- Adopted from TerminalClient repo with Qt5/Qt6 compatibility
- BasicQtApplication template added for Qt app management
- Configuration keys standardized (some legacy keys may differ)
- **2026 Refactoring**: Template implementations moved to `.tpp` file, member variables normalized to camelCase, unused `isPrimaryInstance()` method removed, single-instance handling delegated to SingleApplication library

---

## Further reading

- Implementation & layout: `src/modules/common/application/README.md` (internal structure, build notes)"}]}
