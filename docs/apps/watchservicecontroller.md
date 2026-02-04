# WatchServiceController (Controller Application)

## Overview

WatchServiceController provides a **system tray interface** for monitoring and controlling the WatchService daemon. It offers administrators and users a convenient way to interact with the Self-Service kiosk application without command-line access.

**📖 This document is for administrators and operators** - see [Developer Guide](../../apps/WatchServiceController/README.md) for technical implementation details.

## Purpose

- **Service Management**: Start and stop the WatchService (watchdog) daemon
- **Application Launch**: Launch kiosk application with different scenarios
- **System Integration**: Native system tray integration on all platforms

## Features

### System Tray Interface

The application provides a system tray menu with:

- **Service Control**: Start/stop the WatchService daemon
- **Status Indicators**: Visual connection status (connected/disconnected)
- **Application Launch**: Launch kiosk application with different scenarios

## Configuration

**Note:** WatchServiceController currently does not read configuration files. All settings are hardcoded or use system defaults. Future versions may support configuration files for customization.

## Operation

### Normal Operation

1. **Launch**: Application starts and appears in system tray
2. **Service Control**: Can start/stop the WatchService daemon if needed
3. **Connection**: Automatically connects to WatchService when running
4. **Interaction**: Right-click tray icon for menu access

### Tray Menu Options

- **Start service menu**: Launches the kiosk application with the service menu scenario
- **Start first setup**: Launches the kiosk application with the initial setup scenario
- **Start service**: Launches the kiosk application with web security disabled
- **Stop service**: Stops the WatchService (watchdog) daemon
- **Close**: Closes the controller app

## Logging

### Log Files

Application logs are located at `logs/{yyyy.mm.dd} WatchServiceController.log` in the installation directory, where `{yyyy.mm.dd}` represents the current date in YYYY.MM.DD format.

For example, if installed in `/opt/ekiosk/`, logs would be at `/opt/ekiosk/logs/2026.02.04 WatchServiceController.log`.

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
