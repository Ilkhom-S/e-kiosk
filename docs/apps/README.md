# EKiosk Applications Documentation

This directory contains documentation for all EKiosk applications. Each application serves specific purposes in the kiosk ecosystem.

## Documentation Structure

Each application has **two levels of documentation**:

- **📖 User/Admin Documentation** (`docs/apps/`): Operational guides, configuration, troubleshooting
- **🔧 Developer Documentation** (`apps/{AppName}/README.md`): Technical details, building, development

## Available Applications

### [WatchService (Watchdog Application)](watchservice.md)

Process supervisor and kiosk management daemon for monitoring critical applications.

- **Purpose**: Automatic process recovery, screen protection, health monitoring
- **Key Features**: Module monitoring, automatic restarts, screen protection, priority management
- **Dependencies**: Qt Core, Qt Widgets, BaseApplication, MessageQueue
- **Platform**: Cross-platform (Windows, Linux, macOS)
- **Configuration**: Complex module-based configuration system
- **📖 [User Guide](watchservice.md) | 🔧 [Developer Guide](../../apps/WatchService/README.md)**

### [WatchServiceController (Controller Application)](watchservicecontroller.md)

Controller interface for monitoring and controlling the WatchService daemon.

- **Purpose**: User-friendly interface for WatchService management and monitoring
- **Key Features**: System tray status, manual control, log access, configuration management
- **Dependencies**: Qt Widgets, WatchServiceClient, MessageQueue, SettingsManager
- **Platform**: Cross-platform (Windows, Linux, macOS)
- **Configuration**: Tray behavior and WatchService connection settings
- **📖 [User Guide](watchservicecontroller.md) | 🔧 [Developer Guide](../../apps/WatchServiceController/README.md)**

## Application Organization

Each application documentation includes:

- **Purpose**: What the application does and its role in the system
- **Architecture**: Key components and data flow
- **Configuration**: Complete configuration reference with examples
- **Operation**: How to run and monitor the application
- **Troubleshooting**: Common issues and debugging guidance
- **Integration**: How it interacts with other system components

## Adding New Application Documentation

When adding documentation for a new application:

1. Create documentation following the established pattern
2. Place the .md file in this directory
3. Update this README.md with the new application entry
4. Update the main [configuration reference](../configuration-reference.md) if needed

## Application Categories

- **System Services**: Background services (WatchService, Updater, etc.)
- **User Interfaces**: Main kiosk applications (EKiosk main app)
- **Utilities**: Supporting tools (WatchServiceController, etc.)
- **Maintenance**: Administrative and diagnostic tools

## Related Documentation

- [Modules Documentation](../modules/): Reusable code modules
- [Plugins Documentation](../plugins/): Extensible plugin system
- [Services Documentation](../services/): System services
- [Configuration Reference](../configuration-reference.md): All configuration options
- [Architecture Overview](../architecture.md): System design and components
