# NativeBackend Plugin

## Overview

The NativeBackend plugin provides native QWidget-based graphics rendering for the EKiosk application. It implements the graphics backend interface to render UI components using Qt's native widget system.

## Purpose

This plugin serves as the default graphics backend for EKiosk, providing reliable QWidget-based rendering that works across all supported platforms without requiring additional graphics libraries or Web engines.

## Main Class

**Primary instantiation**: `NativeBackend` class in `src/NativeBackend.cpp`

When the plugin loads, the `CreatePlugin` function (also in `src/NativeBackend.cpp`) instantiates the `NativeBackend` class, which implements the `SDK::GUI::IGraphicsEngine` interface.

## Features

- Native QWidget rendering
- Cross-platform compatibility
- Lightweight implementation
- No external dependencies
- Support for graphics item caching
- Plugin-based graphics item loading

## Configuration

### Plugin Parameters

- **Debug Mode** (`debug`): Enables debug logging for graphics operations
  - Type: Boolean
  - Default: `false`

### Configuration Example

```json
{
  "debug": true
}
```

## Usage

The plugin is automatically loaded by the EKiosk application when the "native" graphics backend is selected. It provides:

- Graphics item creation and management
- Native widget rendering
- Integration with the payment processor core
- Logging through the application's logging system

## Dependencies

- Qt Widgets module
- EKiosk SDK (Plugin, GUI, PaymentProcessor modules)
- No external graphics libraries required

## Build Requirements

- Qt 5.15+ or Qt 6.8+ (depending on platform)
- CMake 3.16+
- C++14 compatible compiler

## Testing

Run the plugin tests using:

```bash
ctest -R native_backend_test
```

Tests cover:

- Plugin loading and factory interface
- Plugin creation and initialization
- Graphics item management
- Configuration handling

## Architecture

The plugin consists of:

- **NativeBackendFactory**: Qt plugin factory implementing `IPluginFactory`
- **NativeBackend**: Main graphics engine implementing `IGraphicsEngine`
- **Registration**: Static registration with the plugin system via `REGISTER_PLUGIN_WITH_PARAMETERS`

## Platform Support

- ✅ Windows 7+ (Qt 5.15+)
- ✅ Windows 10+ (Qt 6.8+)
- ✅ Linux (Qt 6.8+)
- ✅ macOS (planned)

