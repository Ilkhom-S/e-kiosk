# WatchService (Watchdog Application)

## Overview

WatchService is a critical system component that acts as a **process supervisor and kiosk management daemon**. It monitors critical applications, automatically restarts failed processes, and manages screen protection for unattended kiosk operations.

**📖 This document is for administrators and operators** - see [Developer Guide](../../apps/WatchService/README.md) for technical implementation details.

## Purpose

- **Process Monitoring**: Continuously monitors configured modules/applications
- **Automatic Recovery**: Restarts crashed or unresponsive applications
- **Screen Protection**: Displays protective splash screen when no GUI applications are active
- **Health Checks**: Performs periodic health checks via ping messages
- **Priority Management**: Ensures applications start/stop in correct order
- **Single Instance**: Prevents multiple watchdog instances from running

## Architecture

WatchService runs as a background daemon that:

1. **Loads Configuration**: Reads module definitions from `watchdog.ini`
2. **Starts Modules**: Launches configured applications based on priority and dependencies
3. **Monitors Health**: Sends periodic ping messages and monitors responses
4. **Manages Screen**: Shows/hides protective screen based on running GUI applications
5. **Handles Failures**: Automatically restarts failed modules within configured limits

## Configuration

### File Location

- **Primary**: `runtimes/common/data/watchdog/watchdog.ini.in` (template)
- **Runtime**: Generated `watchdog.ini` in build directory
- **User Overrides**: `user.ini` for runtime adjustments

### Module Configuration Parameters

**Important**: Module section names must contain the word "module" (case-insensitive) to be recognized by WatchService. Sections without "module" in the name will be ignored.

Modules are defined in separate INI sections with the patterns shown above. Each module section can contain the following parameters:

#### Basic Module Settings

- **`name`** (string): Module name/identifier (e.g., "ekiosk", "updater")
- **`file`** (string): Executable path (supports `{WS_DIR}` macro for WatchService directory)
- **`workingdirectory`** (string): Working directory for the module (supports `{WS_DIR}` macro)
- **`arguments`** (string): Command line arguments (optional)
- **`gui`** (bool): Whether module has graphical interface that blocks access to desktop and Windows menu

#### Startup Control

- **`autostart`** (bool): Start module automatically on WatchService startup, or manually on command
- **`startmode`** (string):
  - `normal`: Normal startup
  - `service`: Service startup
  - `exclusive`: Requires termination of all other modules
- **`priority`** (int): Startup priority (lower number = starts earlier)
- **`close_priority`** (int): Shutdown priority (lower number = stops earlier)
- **`afterstartdelay`** (int): Delay after startup in milliseconds (optional)

#### Health Monitoring

- **`maxstartcount`** (int): Maximum restart attempts (0 = unlimited restarts, optional)
- **`firstpingtimeout`** (int): Initial ping timeout in seconds (default: 60)
- **`kill_timeout`** (int): Ping interval in seconds (default: 30 for fast PCs, 180 for slow PCs)

#### Main Application Module Example

```ini
[module_ekiosk]
; Basic module information
name = ekiosk
file = {WS_DIR}/ekiosk.exe
workingdirectory = {WS_DIR}
gui = true

; Startup behavior
autostart = true
startmode = normal
priority = 1
close_priority = 1
afterstartdelay = 5000

; Health monitoring
maxstartcount = 3
firstpingtimeout = 60
kill_timeout = 10000
```

#### Updater Service Module Example

```ini
[module_updater]
; Basic module information
name = updater
file = {WS_DIR}/updater.exe
workingdirectory = {WS_DIR}
gui = false

; Startup behavior
autostart = false
startmode = normal
priority = 3
close_priority = 2
afterstartdelay = 0

; Health monitoring
maxstartcount = 1
firstpingtimeout = 60
kill_timeout = 10000
```

#### Priority System

##### Startup Priority

Modules start in ascending priority order (lower numbers first):

```text
priority = 0  → Starts first
priority = 1  → Starts second
priority = 2  → Starts third
```

##### Shutdown Priority

Modules stop in ascending close_priority order:

```text
close_priority = 0  → Stops first
close_priority = 1  → Stops second
```

##### Exclusive Mode

When a module with `startmode = exclusive` runs:

- All other non-exclusive modules are paused
- Exclusive module gets full system resources
- Other modules resume when exclusive module stops

### Forbidden Application Configuration

```ini
[extensions]
watchdog/taboo_enabled = true

[taboo]
applications = notepad.exe,calc.exe,explorer.exe
check_timeout = 60000
```

- **`watchdog/taboo_enabled`** (bool): Enable forbidden app monitoring
- **`applications`** (string list): Comma-separated list of forbidden executables
- **`check_timeout`** (int): Check interval in milliseconds

#### Configuration Variables Notes

WatchService supports variable substitution in configuration values:

- **`{WS_DIR}`**: WatchService current working directory (used for relative paths in file and workingdirectory parameters)

**Example**: `file={WS_DIR}/ekiosk.exe` resolves to the ekiosk executable relative to WatchService directory.

**Note**: For general settings management and configuration file handling, see the [SettingsManager documentation](../../modules/SettingsManager/README.md).

### Screen Protection

WatchService automatically manages screen protection:

- **Protection Active**: Shows splash screen when no GUI modules are running
- **Protection Inactive**: Hides splash screen when GUI modules are active
- **Custom Background**: Configurable via `use_custom_background` setting

## Health Check Mechanism

### Ping Protocol

WatchService communicates with modules via message queue:

1. **Startup Phase**: Waits `firstpingtimeout` seconds for initial response
2. **Monitoring Phase**: Expects ping responses every `killtimeout` seconds
3. **Failure Detection**: Two consecutive missed pings trigger restart
4. **Recovery**: Automatic restart with exponential backoff

### Module States

- **Starting**: Process launched, waiting for first ping
- **Running**: Process responding to pings
- **Failed**: Process not responding, scheduled for restart
- **Stopped**: Process intentionally stopped

## Timing Parameters

| Parameter                   | Default | Description                            |
| --------------------------- | ------- | -------------------------------------- |
| `CheckInterval`             | 3000ms  | Main monitoring loop interval          |
| `ReInitializeTimeout`       | 7000ms  | Delay before reinitialization attempts |
| `ReInitializeFailMaxCount`  | 85      | Max reinitialization attempts          |
| `ContinueExecutionExitCode` | 54321   | Special exit code for continuation     |

## Troubleshooting

### Common Issues

1. **Module Won't Start**
   - Check `file` path and `{WS_DIR}` substitution
   - Verify `workingdirectory` exists
   - Check module dependencies and `priority` settings

2. **Module Keeps Restarting**
   - Check `firstpingtimeout` and `killtimeout` values
   - Verify module responds to ping messages
   - Check `maxstartcount` limit

3. **Screen Protection Not Working**
   - Verify `gui = true` for GUI modules
   - Check module startup and ping response
   - Verify splash screen configuration

4. **Multiple Guard Instances**
   - SingleApplication should prevent this
   - Check for stale shared memory from crashes
   - Verify proper shutdown procedures

### Log Analysis

WatchService logs detailed information about:

- Module startup/shutdown events
- Ping response monitoring
- Screen protection state changes
- Configuration loading
- Error conditions and recovery attempts

### Debug Mode

Enable verbose logging by setting log level in configuration:

```ini
[logging]
level = 0  ; Debug level
```

## Integration Notes

- **Single Instance**: Only one WatchService instance can run per user session
- **Message Queue**: Uses internal message queue for module communication
- **Qt Integration**: Built with Qt framework for cross-platform compatibility
- **Configuration Reload**: Requires restart for configuration changes
- **Resource Management**: Properly manages process lifecycles and cleanup

### Management Interfaces

While WatchService manages core kiosk services, the **WatchServiceController** (controller application) provides a user-friendly system tray interface for:

- **Service Control**: Start/stop the WatchService daemon itself
- **Real-time Monitoring**: Visual status indicators for all modules
- **Manual Control**: Start/stop/restart individual services
- **Log Access**: Quick access to application logs
- **Configuration**: Easy access to settings and config files

**Note**: The controller application connects to WatchService dynamically and is not typically configured as an auto-started module. It should be started separately by administrators when management access is needed.

## Related Documentation

- **🔧 [Developer Guide](../../apps/WatchService/README.md)**: Technical implementation, building, and development details
- **[Configuration Reference](../../configuration-reference.md)**: All configuration options
- **[Modules Documentation](../../modules/)**: Reusable code components
