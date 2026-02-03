# WatchService (Guard Application)

## Overview

WatchService is a critical system component that acts as a **process supervisor and kiosk management daemon**. It monitors critical applications, automatically restarts failed processes, and manages screen protection for unattended kiosk operations.

**📖 This document is for administrators and operators** - see [Developer Guide](../../apps/WatchService/README.md) for technical implementation details.

## Purpose

- **Process Monitoring**: Continuously monitors configured modules/applications
- **Automatic Recovery**: Restarts crashed or unresponsive applications
- **Screen Protection**: Displays protective splash screen when no GUI applications are active
- **Health Checks**: Performs periodic health checks via ping messages
- **Priority Management**: Ensures applications start/stop in correct order
- **Single Instance**: Prevents multiple guard instances from running

## Architecture

WatchService runs as a background daemon that:

1. **Loads Configuration**: Reads module definitions from `WatchService.ini`
2. **Starts Modules**: Launches configured applications based on priority and dependencies
3. **Monitors Health**: Sends periodic ping messages and monitors responses
4. **Manages Screen**: Shows/hides protective screen based on running GUI applications
5. **Handles Failures**: Automatically restarts failed modules within configured limits

## Configuration

### File Location

- **Primary**: `apps/WatchService/src/WatchService.ini` (template)
- **Runtime**: Generated `WatchService.ini` in build directory
- **User Overrides**: `user.ini` for runtime adjustments

### Module Configuration Format

Each module is defined in a separate INI section with the pattern `[module_<name>]`:

```ini
[module_payment_processor]
; Basic module information
name = payment_processor
file = {WS_DIR}/payment_processor${EXE_SUFFIX}
workingdirectory = {WS_DIR}

; Startup behavior
autostart = true
startmode = normal
priority = 1
close_priority = 0
afterstartdelay = 3000

; Health monitoring
maxstartcount = 0
firstpingtimeout = 60
killtimeout = 30

; UI behavior
gui = true
```

### Configuration Parameters

#### Basic Module Settings

- **`name`** (string): Unique module identifier
- **`file`** (string): Executable path (supports `{WS_DIR}` and `${EXE_SUFFIX}` variables for cross-platform compatibility)
- **`workingdirectory`** (string): Working directory for the module
- **`arguments`** (string): Command line arguments (optional)

#### Startup Control

- **`autostart`** (bool): Start module automatically on WatchService startup
- **`startmode`** (string):
  - `normal`: Standard startup
  - `service`: Run as background service
  - `exclusive`: Exclusive mode (pauses other modules)
- **`priority`** (int): Startup priority (lower = starts first)
- **`close_priority`** (int): Shutdown priority (lower = stops first)
- **`afterstartdelay`** (int): Delay after startup in milliseconds

#### Health Monitoring

- **`maxstartcount`** (int): Maximum restart attempts (0 = unlimited)
- **`firstpingtimeout`** (int): Initial ping timeout in seconds (default: 60)
- **`killtimeout`** (int): Ping interval in seconds (default: 30)
- **`needtostart`** (bool): Runtime control flag

### Configuration Variables

WatchService supports variable substitution in configuration values:

- **`{WS_DIR}`**: WatchService working directory
- **`${EXE_SUFFIX}`**: Platform-specific executable suffix (`.exe` on Windows, empty on Unix)

#### UI Integration

- **`gui`** (bool): Module has graphical interface

### Special Features

#### Screen Protection

WatchService automatically manages screen protection:

- **Protection Active**: Shows splash screen when no GUI modules are running
- **Protection Inactive**: Hides splash screen when GUI modules are active
- **Custom Background**: Configurable via `use_custom_background` setting

#### Forbidden Application Monitoring

```ini
[extensions]
guard/taboo_enabled = true

[taboo]
applications = notepad${EXE_SUFFIX},calc${EXE_SUFFIX},explorer${EXE_SUFFIX}
check_timeout = 60000
```

- **`guard/taboo_enabled`** (bool): Enable forbidden app monitoring
- **`applications`** (string list): Comma-separated list of forbidden executables
- **`check_timeout`** (int): Check interval in milliseconds

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

## Priority System

### Startup Priority

Modules start in ascending priority order (lower numbers first):

```text
priority = 0  → Starts first
priority = 1  → Starts second
priority = 2  → Starts third
```

### Shutdown Priority

Modules stop in ascending close_priority order:

```text
close_priority = 0  → Stops first
close_priority = 1  → Stops second
```

### Exclusive Mode

When a module with `startmode = exclusive` runs:

- All other non-exclusive modules are paused
- Exclusive module gets full system resources
- Other modules resume when exclusive module stops

## Timing Parameters

| Parameter                   | Default | Description                            |
| --------------------------- | ------- | -------------------------------------- |
| `CheckInterval`             | 3000ms  | Main monitoring loop interval          |
| `ReInitializeTimeout`       | 7000ms  | Delay before reinitialization attempts |
| `ReInitializeFailMaxCount`  | 85      | Max reinitialization attempts          |
| `ContinueExecutionExitCode` | 54321   | Special exit code for continuation     |

## Configuration Examples

### Basic Payment Processor

```ini
[module_payment_processor]
name = payment_processor
file = {WS_DIR}/payment_processor${EXE_SUFFIX}
workingdirectory = {WS_DIR}
autostart = true
startmode = normal
priority = 1
close_priority = 0
afterstartdelay = 3000
gui = true
firstpingtimeout = 60
killtimeout = 30
maxstartcount = 0
```

### Background Database Service

```ini
[module_mysql]
name = MYSQL
file = {WS_DIR}/mysql${EXE_SUFFIX}
workingdirectory = {WS_DIR}
autostart = true
startmode = service
priority = 0
close_priority = 3
afterstartdelay = 5000
gui = false
firstpingtimeout = 120
killtimeout = 60
maxstartcount = 3
```

### Exclusive Maintenance Mode

```ini
[module_maintenance]
name = maintenance
file = {WS_DIR}/maintenance${EXE_SUFFIX}
workingdirectory = {WS_DIR}
autostart = false
startmode = exclusive
priority = 10
close_priority = 0
afterstartdelay = 1000
gui = true
firstpingtimeout = 30
killtimeout = 15
maxstartcount = 1
```

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

## Related Documentation

- **🔧 [Developer Guide](../../apps/WatchService/README.md)**: Technical implementation, building, and development details
- **[Configuration Reference](../../configuration-reference.md)**: All configuration options
- **[Modules Documentation](../../modules/)**: Reusable code components
