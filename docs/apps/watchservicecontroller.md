# WatchServiceController (Controller Application)

## Overview

WatchServiceController provides a **system tray interface** for monitoring and controlling the WatchService daemon. It offers administrators and users a convenient way to interact with the kiosk management system without command-line access.

**📖 This document is for administrators and operators** - see [Developer Guide](../../apps/WatchServiceController/README.md) for technical implementation details.

## Purpose

- **Service Management**: Start and stop the WatchService (watchdog) daemon
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

- **Primary**: `apps/WatchServiceController/src/controller.ini` (template)
- **Runtime**: Generated `controller.ini` in build directory
- **User Overrides**: `user.ini` for runtime adjustments

### Configuration Options

```ini
[Controller]
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
| Controller   | AutoStart           | bool   | true      | Auto-start with system         |
| Controller   | ShowNotifications   | bool   | true      | Show system notifications      |
| Controller   | MinimizeOnClose     | bool   | true      | Minimize to tray on close      |
| Display      | IconTheme           | string | default   | Icon theme to use              |
| Display      | ShowStatusInMenu    | bool   | true      | Show status in menu            |
| Display      | NotificationTimeout | int    | 5000      | Notification display time (ms) |
| WatchService | ServiceHost         | string | localhost | WatchService host              |
| WatchService | ServicePort         | int    | 8080      | WatchService port              |
| WatchService | ConnectionTimeout   | int    | 30000     | Connection timeout (ms)        |

## Operation

### Normal Operation

1. **Launch**: Application starts and appears in system tray
2. **Service Control**: Can start/stop the WatchService daemon if needed
3. **Connection**: Automatically connects to WatchService when running
4. **Monitoring**: Displays real-time status of all modules
5. **Interaction**: Right-click tray icon for menu access

### Tray Menu Options

- **Start Service**: Launch the WatchService (watchdog) daemon
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

#### Controller Icon Not Visible

**Symptoms:** Controller icon doesn't appear in system tray

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

- **Windows**: `%APPDATA%\EKiosk\controller.log`
- **Linux**: `~/.local/share/EKiosk/controller.log`
- **macOS**: `~/Library/Application Support/EKiosk/controller.log`

### Debug Mode

Enable debug logging by setting:

```ini
[Logging]
Level=debug
File=tray_debug.log
```

## Related Documentation

- **🔧 [Developer Guide](../../apps/WatchServiceController/README.md)**: Technical implementation details
- **[WatchService User Guide](watchservice.md)**: Main service documentation
- **[Configuration Reference](../configuration-reference.md)**: Complete configuration options
- **[Troubleshooting Guide](../troubleshooting.md)**: General troubleshooting
