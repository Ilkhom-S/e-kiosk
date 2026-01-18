# WebEngineBackend Plugin

Qt WebEngine-based graphics rendering backend for EKiosk applications.

## Overview

The WebEngineBackend plugin provides Qt WebEngine-based graphics rendering capabilities for EKiosk kiosk applications. It implements the `IGraphicsBackend` interface to enable modern web-based UI development using Chromium-based rendering.

**Note**: This plugin is optimized for Qt 6.x. For Qt 5.x compatibility, use the WebKitBackend plugin instead.

## Features

- **Qt WebEngine Rendering**: Full Chromium-based rendering
- **HTML/CSS/JavaScript Support**: Modern web technologies
- **Plugin Architecture**: Implements EKiosk plugin system interfaces
- **Cross-Platform**: Works on Windows, Linux, and macOS
- **Qt6 Compatible**: Optimized for Qt 6.x environments

## Architecture

### Core Components

- **WebEngineBackend**: Main plugin class implementing `IPlugin`, `IGraphicsBackend`, and containing plugin registration
- **WebEngineBackendFactory**: Plugin factory base class with static metadata
- **WebGraphicsItem**: WebEngine-based graphics item wrapper
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
  "graphicsBackend": "webengine"
}
```

## Dependencies

### Qt Modules

- Core, Widgets, WebEngineCore, WebEngineWidgets (Qt 6.x)
- Network (for web content loading)

### EKiosk SDK

- Plugin interfaces (`IPlugin`, `IPluginFactory`)
- GUI interfaces (`IGraphicsBackend`)
- Payment Processor interfaces (`ICore`, `Scripting::Core`)

## File Structure

```text
WebEngineBackend/
├── CMakeLists.txt           # Build configuration
└── src/
    ├── WebEngineBackend.cpp     # Main plugin entry point - graphics backend implementation
    ├── WebEngineBackend.h       # Main plugin header
    ├── WebEngineBackendFactory.cpp # Plugin metadata and factory base class
    ├── WebEngineBackendFactory.h  # Plugin factory header
    ├── WebGraphicsItem.cpp     # WebEngine graphics wrapper
    ├── WebGraphicsItem.h       # WebEngine graphics header
    ├── WebPageLogger.cpp       # Web page logging functionality
    └── WebPageLogger.h         # Web page logger header
```

**Main Entry Point**: `WebEngineBackend.cpp` contains the `REGISTER_PLUGIN_WITH_PARAMETERS` macro and `CreatePlugin` function that registers the plugin with the EKiosk system.

**Metadata**: `WebEngineBackendFactory.cpp` defines the static metadata (name, description, version, author) used by the plugin system.

## Building

The plugin is built using the EKiosk CMake system:

```cmake
ek_add_plugin(webengine_backend
    FOLDER "plugins/GraphicBackends"
    SOURCES ${WEBENGINEBACKEND_SOURCES}
    QT_MODULES Core Widgets WebEngineCore WebEngineWidgets Network
)
```

## Testing

Run plugin tests:

```bash
ctest -R webengine_backend_test
```

Or run all plugin tests:

```bash
ctest -R webengine
```

Tests cover:

- Plugin loading and factory interface
- WebEngine engine creation and web content rendering
- Graphics backend functionality
- Mock kernel integration

## Usage Examples

### Basic Plugin Usage

```cpp
// Plugin is loaded automatically by kernel
auto kernel = getKernel();
auto graphicsBackend = kernel->getInterface<SDK::GUI::IGraphicsBackend>();

// Create WebEngine-based UI
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

1. **WebEngine Module Not Found**
   - Ensure Qt WebEngine installation
   - Check Qt version compatibility (Qt 6.x recommended)

2. **Plugin Loading Failure**
   - Verify plugin registration in `WebEngineBackend.cpp`
   - Check plugin directory in build output

3. **Web Content Not Loading**
   - Validate HTML/CSS/JavaScript syntax
   - Check network connectivity for remote content

## Migration Notes

This plugin has been migrated to follow the new EKiosk plugin architecture:

- **Qt5/Qt6 Compatibility**: Optimized for Qt 6.x with WebEngine
- **Service Integration**: Now properly integrates with EKiosk services
- **Plugin Architecture**: Follows standard IPluginFactory/IPlugin pattern
- **Code Standards**: Updated to follow EKiosk coding standards with Russian comments
- **Registration Refactor**: Main entry point moved to `WebEngineBackend.cpp` with `REGISTER_PLUGIN_WITH_PARAMETERS` macro, metadata centralized in `WebEngineBackendFactory.cpp`

## Platform Support

- ✅ Windows 10+ (Qt 6.8+)
- ✅ Linux (Qt 6.8+)
- ✅ macOS (Qt 6.8+)
- ❌ Windows 7 (Qt 5.15 - use WebKitBackend instead)

## See Also

- [Plugin System Architecture](../../../.github/copilot-instructions.md#plugin-system-architecture)
- [WebKitBackend Plugin](../WebKitBackend/README.md)
- [QMLBackend Plugin](../QMLBackend/README.md)
- [NativeBackend Plugin](../NativeBackend/README.md)
