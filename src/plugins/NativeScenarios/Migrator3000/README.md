# Migrator3000 Scenario Plugin

## Overview

The Migrator3000 plugin provides a native scenario for automatic migration from EKiosk version 2.x.x to 3.x.x. This plugin handles the complex migration process including data transformation, configuration updates, and system state transitions.

## Purpose

This plugin enables seamless upgrades between major EKiosk versions by automating the migration of:

- User configurations and settings
- Database schemas and data
- Device configurations
- Payment processor settings
- Scenario definitions and scripts

## Features

- **Automated Migration**: Handles version-specific migration logic
- **Data Integrity**: Ensures data consistency during migration
- **Rollback Support**: Provides recovery mechanisms for failed migrations
- **Progress Tracking**: Reports migration progress and status
- **Error Handling**: Comprehensive error reporting and recovery

## Platform Support

| Platform | Status  | Notes                      |
| -------- | ------- | -------------------------- |
| Windows  | ✅ Full | Supported on Windows 7+    |
| Linux    | ✅ Full | Full migration support     |
| macOS    | 🔬 TODO | Planned for future release |

## Configuration

The plugin uses the following configuration options:

- **Migration Source**: Source version and data location
- **Migration Target**: Target version specifications
- **Backup Settings**: Automatic backup before migration
- **Validation Options**: Post-migration data validation

## Usage

The plugin is automatically loaded during EKiosk startup when a migration is detected. Manual triggering is also supported through the administration interface.

### Integration

The plugin integrates with:

- Payment Processor SDK (PPSDK)
- Database services
- Configuration management
- Device management
- Network services

## Dependencies

- **PPSDK**: Payment Processor SDK for terminal and payment services
- **ScenarioEngine**: JavaScript scenario execution engine
- **QtScript**: For script-based migration logic
- **QtQml**: For advanced scripting capabilities

## Testing

Tests are located in `tests/plugins/NativeScenarios/Migrator3000/` and include:

- Unit tests for migration logic
- Integration tests with PPSDK
- Scenario execution tests
- Error handling and recovery tests

Run tests with:

```bash
cmake --build build/msvc --target migrator3000_scenario_test
ctest -R migrator3000_scenario_test
```

## Implementation Details

### Main Classes

- **MainScenario**: Core migration scenario implementation
- **MainScenarioPlugin**: Plugin factory and lifecycle management
- **PluginFactoryDefinition**: Static metadata configuration

### File Structure

```
src/
├── MainScenario.cpp/.h          # Main migration scenario logic
├── PluginFactoryDefinition.cpp  # Static plugin metadata
└── ScenarioPlugin.h             # Plugin interface implementation
```

## Migration Notes

- Requires exclusive access to database during migration
- Automatic backup creation before migration starts
- Supports incremental migration for large datasets
- Compatible with Qt 5.15+ and Qt 6.8+ versions

