# WatchServiceController (Controller Application)

## Overview

WatchServiceController provides a **system tray interface** for monitoring and controlling the WatchService daemon. It offers administrators and users a convenient way to interact with the Self-Service kiosk application without command-line access.

**📖 This document is for administrators and operators** - see [Developer Guide](../../apps/WatchServiceController/README.md) for technical implementation details.

## Purpose

- **Service Management**: Start and stop the WatchService (watchdog) daemon
- **Status Monitoring**: Real-time display of WatchService and module status
- **System Integration**: Native system tray integration on all platforms

## Features

### System Tray Interface

The application provides a comprehensive system tray menu with:

- **Service Control**: Start/stop the WatchService daemon
- **Status Indicators**: Visual status of WatchService and monitored applications
- **Application Control**: Start/stop/restart individual modules
- **System Information**: Version information and system status

## Configuration

**Note:** WatchServiceController currently does not read configuration files. All settings are hardcoded or use system defaults. Future versions may support configuration files for customization.

## Operation

### Normal Operation

1. **Launch**: Application starts and appears in system tray
2. **Service Control**: Can start/stop the WatchService daemon if needed
3. **Connection**: Automatically connects to WatchService when running
4. **Monitoring**: Displays real-time status of all modules
5. **Interaction**: Right-click tray icon for menu access

### Tray Menu Options

- **Start/Stop WatchdogService**: Start or stop the WatchdogService (watchdog) daemon
- **Start service menu**: Opens kiosk service menu screen
- **Start first setup**: Starts kiosk for initial setup
- **Close**: Closes the controller app

## Related Documentation

- **🔧 [Developer Guide](../../apps/WatchServiceController/README.md)**: Technical implementation details
- **[WatchService User Guide](watchservice.md)**: Main service documentation
- **[Configuration Reference](../configuration-reference.md)**: Complete configuration options
- **[Troubleshooting Guide](../troubleshooting.md)**: General troubleshooting
