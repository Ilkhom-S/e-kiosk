# WatchServiceController (Tray Application)

## Overview

WatchServiceController provides a **system tray interface** for monitoring and controlling the WatchService daemon. It offers administrators and users a convenient way to interact with the kiosk management system without command-line access.

**📖 This document is for administrators and operators** - see [Developer Guide](../../apps/WatchServiceController/README.md) for technical implementation details.

## Purpose

- **Service Management**: Start and stop the WatchService (guard) daemon
- **Status Monitoring**: Real-time display of WatchService and module status
- **Manual Control**: Start, stop, and restart applications manually
- **Log Access**: Quick access to application logs and error messages
- **Configuration**: Easy access to settings and configuration files
- **System Integration**: Native system tray integration on all platforms

## Features

### System Tray Interface

The application provides a comprehensive system tray menu with:

- **Service Control**: Start/stop the WatchService daemon
- **Status Indicators**: Visual status of WatchService and monitored applications
- **Application Control**: Start/stop/restart individual modules
- **Log Viewer**: Access to application logs and error messages
- **Configuration Access**: Open configuration files and settings
- **System Information**: Version information and system status

### Status Indicators

| Icon | Status   | Description                       |
| ---- | -------- | --------------------------------- |
| 🟢   | Running  | Application is running normally   |
| 🔴   | Stopped  | Application is stopped            |
| 🟡   | Starting | Application is starting up        |
| ⚠️   | Error    | Application has errors or crashed |

## Configuration

### File Location

- **Primary**: `apps/WatchServiceController/src/tray.ini` (template)
- **Runtime**: Generated `tray.ini` in build directory
- **User Overrides**: `user.ini` for runtime adjustments

### Configuration Options

```ini
[Tray]
; Auto-start behavior
AutoStart=true
ShowNotifications=true
MinimizeOnClose=true

[Display]
; UI customization
IconTheme=default
ShowStatusInMenu=true
NotificationTimeout=5000

[WatchService]
; Connection settings
ServiceHost=localhost
ServicePort=8080
ConnectionTimeout=30000
```

### Configuration Parameters

| Section      | Parameter           | Type   | Default   | Description                    |
| ------------ | ------------------- | ------ | --------- | ------------------------------ |
| Tray         | AutoStart           | bool   | true      | Auto-start with system         |
| Tray         | ShowNotifications   | bool   | true      | Show system notifications      |
| Tray         | MinimizeOnClose     | bool   | true      | Minimize to tray on close      |
| Display      | IconTheme           | string | default   | Icon theme to use              |
| Display      | ShowStatusInMenu    | bool   | true      | Show status in menu            |
| Display      | NotificationTimeout | int    | 5000      | Notification display time (ms) |
| WatchService | ServiceHost         | string | localhost | WatchService host              |
| WatchService | ServicePort         | int    | 8080      | WatchService port              |
| WatchService | ConnectionTimeout   | int    | 30000     | Connection timeout (ms)        |

## Installation and Startup

### System Integration

**Windows:**

- Add to Startup folder for auto-start
- Can be configured as system service

**Linux:**

- Add desktop file to `~/.config/autostart/`
- Use systemd user service

**macOS:**

- Add to Login Items in System Preferences
- Use launchd user agent

### Manual Startup

```bash
# From build directory
./WatchServiceController

# With options
./WatchServiceController --minimized --no-autostart
```

## Operation

### Normal Operation

1. **Launch**: Application starts and appears in system tray
2. **Service Control**: Can start/stop the WatchService daemon if needed
3. **Connection**: Automatically connects to WatchService when running
4. **Monitoring**: Displays real-time status of all modules
5. **Interaction**: Right-click tray icon for menu access

### Tray Menu Options

- **Start Service**: Launch the WatchService (guard) daemon
- **Start Application**: Launch specific module
- **Stop Application**: Stop specific module
- **Restart Application**: Restart specific module
- **View Logs**: Open log viewer for specific module
- **Open Config Folder**: Open configuration directory
- **Settings**: Open configuration editor
- **About**: Show version and system information
- **Exit**: Close the tray application

## Troubleshooting

### Common Issues

#### Tray Icon Not Visible

**Symptoms:** Tray icon doesn't appear in system tray

**Solutions:**

- Check if system tray is enabled in desktop environment
- Verify Qt platform plugins are installed
- Check application logs for errors

#### Cannot Connect to WatchService

**Symptoms:** Status shows "Disconnected", cannot control applications

**Solutions:**

- Verify WatchService is running
- Check network connectivity
- Verify connection settings in configuration
- Check firewall settings

#### High CPU Usage

**Symptoms:** Application uses excessive CPU

**Solutions:**

- Check for infinite loops in status updates
- Verify Qt event loop is functioning
- Check for memory leaks

### Log Files

Application logs are typically located in:

- **Windows**: `%APPDATA%\EKiosk\tray.log`
- **Linux**: `~/.local/share/EKiosk/tray.log`
- **macOS**: `~/Library/Application Support/EKiosk/tray.log`

### Debug Mode

Enable debug logging by setting:

```ini
[Logging]
Level=debug
File=tray_debug.log
```

## Integration with WatchService

### Communication Protocol

The tray application communicates with WatchService using:

- **Message Queue**: Qt signals/slots for IPC
- **Status Updates**: Real-time status notifications
- **Command Execution**: Remote start/stop/restart commands
- **Health Monitoring**: Ping/pong health checks

### Module Management

The tray provides fine-grained control over individual modules:

- **Individual Control**: Start/stop specific modules without affecting others
- **Dependency Awareness**: Respects module dependencies and priorities
- **Status Tracking**: Real-time status updates for all modules
- **Error Reporting**: Detailed error messages and recovery suggestions

### Optional Auto-Start Configuration

While the tray application is primarily a user interface tool and not typically auto-started by WatchService, it can be configured as a module for environments requiring automatic tray availability:

```ini
[module_tray_controller]
name = tray_controller
file = {WS_DIR}/tray${EXE_SUFFIX}
workingdirectory = {WS_DIR}
autostart = true
startmode = normal
priority = 10
close_priority = 0
gui = true
firstpingtimeout = 30
killtimeout = 60
maxstartcount = 3
```

**Note**: Auto-starting the tray app is optional and typically not recommended for production kiosk deployments where the interface should only be available to administrators.

## Platform-Specific Notes

### Windows

- Full system tray support
- Native Windows notifications
- Integration with Windows Task Manager
- Startup folder integration

### Linux

- Depends on desktop environment tray support
- May require specific Qt platform plugins
- Systemd user service integration
- X11/Wayland compatibility

### macOS

- Native macOS menu bar integration
- Notification Center integration
- Launchd user agent support
- App bundle support

## Security Considerations

- **IPC Security**: Communication with WatchService should be secured
- **Configuration Access**: Sensitive settings should be protected
- **Log Security**: Log files may contain sensitive information
- **Network Security**: Remote connections should use secure protocols

## Performance Tuning

### Memory Usage

- Monitor memory usage in long-running deployments
- Configure appropriate log rotation
- Clean up unused resources

### CPU Usage

- Status update frequency can be adjusted
- Notification frequency can be limited
- Background processing can be optimized

### Network Usage

- Connection keep-alive settings
- Timeout configurations
- Retry logic for network failures

## Related Documentation

- **🔧 [Developer Guide](../../apps/WatchServiceController/README.md)**: Technical implementation details
- **[WatchService User Guide](watchservice.md)**: Main service documentation
- **[Configuration Reference](../configuration-reference.md)**: Complete configuration options
- **[Troubleshooting Guide](../troubleshooting.md)**: General troubleshooting
