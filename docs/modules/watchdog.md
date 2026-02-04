# Watchdog Service

EKiosk watchdog service for monitoring and managing application modules with automatic restart capabilities and forbidden application blocking.

## Purpose

The Watchdog service monitors configured application modules, automatically restarts failed processes, manages startup/shutdown priorities, and blocks forbidden applications from running.

## Quick Start

```ini
[Module1]
name=ekiosk
file={WS_DIR}/../../ekiosk.exe
workingdirectory={WS_DIR}
startmode=auto
autostart=true
priority=1
close_priority=1
maxstartcount=3
gui=true
firstpingtimeout=30000
kill_timeout=10000

[taboo]
applications=explorer.exe,taskmgr.exe
check_timeout=60000
```

## Features

- **Module Management**: Automatic startup, monitoring, and restart of configured modules
- **Priority Control**: Configurable startup and shutdown priorities for modules
- **Process Monitoring**: Health checking via ping mechanism with configurable timeouts
- **Forbidden Application Blocking**: Prevents specified applications from running
- **Cross-Platform**: Supports Windows, macOS, and Linux with platform-specific configurations

## Platform Support

| Platform | Status  | Notes                                                        |
| -------- | ------- | ------------------------------------------------------------ |
| Windows  | ✅ Full | Complete support with process enumeration and taboo blocking |
| Linux    | ✅ Full | Full support with process monitoring                         |
| macOS    | ✅ Full | Full support with application monitoring                     |

## Configuration

### General Section [Общие]

| Parameter     | Type   | Default | Description                           |
| ------------- | ------ | ------- | ------------------------------------- |
| LOG_PATH      | string | -       | Path to log file directory            |
| CHECK_TIMEOUT | int    | 60000   | General check timeout in milliseconds |

### Taboo Section [taboo]

| Parameter     | Type   | Default | Description                                               |
| ------------- | ------ | ------- | --------------------------------------------------------- |
| applications  | string | -       | Comma-separated list of forbidden application executables |
| check_timeout | int    | 60000   | Timeout between taboo application checks in milliseconds  |

### Module Sections [Module1], [module_updater], etc

| Parameter        | Type   | Default  | Description                                               |
| ---------------- | ------ | -------- | --------------------------------------------------------- |
| name             | string | -        | Unique module identifier                                  |
| file             | string | -        | Path to executable file (supports {WS_DIR} placeholder)   |
| workingdirectory | string | {WS_DIR} | Working directory for the module                          |
| startmode        | string | auto     | Startup mode: "auto" or "manual"                          |
| autostart        | bool   | false    | Whether to start module automatically on watchdog startup |
| priority         | int    | 1        | Startup priority (lower = higher priority)                |
| close_priority   | int    | 1        | Shutdown priority (lower = higher priority)               |
| afterstartdelay  | int    | 0        | Delay after successful start in milliseconds              |
| maxstartcount    | int    | 3        | Maximum restart attempts on failure                       |
| gui              | bool   | false    | Whether module has graphical interface                    |
| firstpingtimeout | int    | 30000    | Timeout for first ping response in milliseconds           |
| kill_timeout     | int    | 10000    | Timeout for forced process termination in milliseconds    |

## Usage

The watchdog reads its configuration from `watchdog.ini` in the same directory. Modules are identified by sections containing "module" in the name (case-insensitive).

### Module Lifecycle

1. **Startup**: Modules with `autostart=true` are started in priority order
2. **Monitoring**: Health checks via ping mechanism
3. **Restart**: Automatic restart on failure up to `maxstartcount`
4. **Shutdown**: Graceful shutdown in reverse priority order

### Taboo Application Blocking

When enabled, the watchdog periodically checks for and terminates forbidden applications listed in the `taboo` section.

## Integration

The watchdog is built as part of the EKiosk application suite using CMake:

```cmake
ek_add_application(watchdog
    FOLDER "apps"
    SOURCES ${WATCHSERVICE_SOURCES}
    QT_MODULES Gui Widgets
    DEPENDS Log SysUtils DebugUtils SettingsManager MessageQueue PPSDK
)
```

## Testing

Tests are located in `tests/apps/WatchService/` and include:

- Module startup/shutdown testing
- Process monitoring verification
- Configuration parsing validation
- Cross-platform compatibility tests

Run tests with: `ctest -R WatchService`

## Migration Notes

- Configuration format is backward compatible with Terminal Client guard app
- Qt5/Qt6 compatible with automatic API selection
- Platform-specific paths handled via merged template system

## Further Reading

- [Build Guide](../build-guide.md) - Building and configuring EKiosk
- [Configuration Reference](../configuration-reference.md) - General configuration patterns
- [Platform Compatibility](../platform-compatibility.md) - Platform-specific considerations
