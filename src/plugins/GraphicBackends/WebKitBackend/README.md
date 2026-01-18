# WebKitBackend Plugin

Qt WebKit-based graphics rendering backend for EKiosk applications.

## Overview

The WebKitBackend plugin provides Qt WebKit-based graphics rendering capabilities for EKiosk kiosk applications. It implements the `IGraphicsBackend` interface to enable web-based UI development using Qt's WebKit engine.

**Note**: This plugin is primarily for Qt 5.x compatibility. For Qt 6.x, use the WebEngineBackend plugin instead.

## Features

- **Qt WebKit Rendering**: Full WebKit engine rendering
- **HTML/CSS/JavaScript Support**: Modern web technologies
- **Plugin Architecture**: Implements EKiosk plugin system interfaces
- **Cross-Platform**: Works on Windows, Linux, and macOS
- **Qt5 Compatible**: Optimized for Qt 5.x environments

## Architecture

### Core Components

- **WebKitBackend**: Main plugin class implementing `IPlugin`, `IGraphicsBackend`, and containing plugin registration
- **WebKitBackendFactory**: Plugin factory base class with static metadata
- **WebGraphicsItem**: WebKit-based graphics item wrapper
- **WebPageLogger**: Web page event logging functionality

### Interfaces Implemented

- `SDK::Plugin::IPlugin`: Plugin lifecycle management
- `SDK::Plugin::IPluginFactory`: Plugin creation and metadata
- `SDK::GUI::IGraphicsBackend`: Graphics rendering backend
- `SDK::PaymentProcessor::Core::ICore`: Payment processing integration

## Configuration

The plugin supports configuration through the EKiosk plugin system:

```json
{
  "webImportPaths": ["/custom/web/modules"],
  "scriptEngine": "enabled",
  "graphicsBackend": "webkit"
}
```

## Dependencies

### Qt Modules

- Core, Widgets, WebKit, WebKitWidgets
- Network (for web content loading)

### EKiosk SDK

- Plugin interfaces (`IPlugin`, `IPluginFactory`)
- GUI interfaces (`IGraphicsBackend`)
- Payment Processor interfaces (`ICore`, `Scripting::Core`)

## File Structure

```text
WebKitBackend/
├── CMakeLists.txt           # Build configuration
└── src/
    ├── WebKitBackend.cpp       # Main plugin entry point - graphics backend implementation
    ├── WebKitBackend.h         # Main plugin header
    ├── WebKitBackendFactory.cpp # Plugin metadata and factory base class
    ├── WebKitBackendFactory.h  # Plugin factory header
    ├── WebGraphicsItem.cpp     # WebKit graphics wrapper
    ├── WebGraphicsItem.h       # WebKit graphics header
    ├── WebPageLogger.cpp       # Web page logging functionality
    └── WebPageLogger.h         # Web page logger header
```

**Main Entry Point**: `WebKitBackend.cpp` contains the `REGISTER_PLUGIN_WITH_PARAMETERS` macro and `CreatePlugin` function that registers the plugin with the EKiosk system.

**Metadata**: `WebKitBackendFactory.cpp` defines the static metadata (name, description, version, author) used by the plugin system.

## Building

The plugin is built using the EKiosk CMake system:

```cmake
ek_add_plugin(webkit_backend
    FOLDER "plugins/GraphicBackends"
    SOURCES ${WEBKITBACKEND_SOURCES}
    QT_MODULES Core Widgets WebKit WebKitWidgets Network
)
```

## Testing

Run plugin tests:

```bash
ctest -R webkit_backend_test
```

Or run all plugin tests:

```bash
ctest -R webkit
```

Tests cover:

- Plugin loading and factory interface
- WebKit engine creation and web content rendering
- Graphics backend functionality
- Mock kernel integration

## Usage Examples

### Basic Plugin Usage

```cpp
// Plugin is loaded automatically by kernel
auto kernel = getKernel();
auto graphicsBackend = kernel->getInterface<SDK::GUI::IGraphicsBackend>();

// Create WebKit-based UI
auto mainWindow = graphicsBackend->createGraphicsItem("html/MainWindow.html");
```

### Web Content Integration

```cpp
// Load web-based payment interface
auto webItem = graphicsBackend->createGraphicsItem("payment.html");
webItem->loadContent(paymentHtmlContent);
```

## Troubleshooting

### Common Issues

1. **WebKit Module Not Found**
   - Ensure Qt WebKit installation
   - Check Qt version compatibility (Qt 5.x required)

2. **Plugin Loading Failure**
   - Verify plugin registration in `WebKitBackend.cpp`
   - Check plugin directory in build output

3. **Web Content Not Loading**
   - Validate HTML/CSS/JavaScript syntax
   - Check network connectivity for remote content

## Migration Notes

This plugin has been migrated to follow the new EKiosk plugin architecture:

- **Qt5/Qt6 Compatibility**: Optimized for Qt 5.x with WebKit
- **Service Integration**: Now properly integrates with EKiosk services
- **Plugin Architecture**: Follows standard IPluginFactory/IPlugin pattern
- **Code Standards**: Updated to follow EKiosk coding standards with Russian comments
- **Registration Refactor**: Main entry point moved to `WebKitBackend.cpp` with `REGISTER_PLUGIN_WITH_PARAMETERS` macro, metadata centralized in `WebKitBackendFactory.cpp`

## Platform Support

- ✅ Windows 7+ (Qt 5.15+)
- ✅ Linux (Qt 5.15+)
- ❌ macOS (Qt WebKit deprecated)
- ❌ Windows 10+ Qt 6.x (use WebEngineBackend instead)

## See Also

- [Plugin System Architecture](../../../.github/copilot-instructions.md#plugin-system-architecture)
- [WebEngineBackend Plugin](../WebEngineBackend/README.md)
- [QMLBackend Plugin](../QMLBackend/README.md)
- [NativeBackend Plugin](../NativeBackend/README.md)
