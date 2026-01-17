# QMLBackend Plugin

Qt QML-based graphics rendering backend for EKiosk applications.

## Overview

The QMLBackend plugin provides Qt QML and Qt Quick-based graphics rendering capabilities for EKiosk kiosk applications. It implements the `IGraphicsBackend` interface to enable modern declarative UI development using QML.

## Features

- **Qt QML Rendering**: Full Qt Quick scene graph rendering
- **Scripting Integration**: Payment processor script execution support
- **Plugin Architecture**: Implements EKiosk plugin system interfaces
- **Cross-Platform**: Works on Windows, Linux, and macOS
- **Qt5/Qt6 Compatible**: Automatic version detection and module selection

## Architecture

### Core Components

- **QMLBackendFactory**: Plugin factory implementing `IPluginFactory`
- **QMLBackend**: Main plugin class implementing `IPlugin` and `IGraphicsBackend`
- **QMLGraphicsItem**: QML-based graphics item wrapper
- **Md5ValidatorQmlItem**: QML item for MD5 validation

### Interfaces Implemented

- `SDK::Plugin::IPlugin`: Plugin lifecycle management
- `SDK::Plugin::IPluginFactory`: Plugin creation and metadata
- `SDK::GUI::IGraphicsBackend`: Graphics rendering backend
- `SDK::PaymentProcessor::Core::ICore`: Payment processing integration

## Configuration

The plugin supports configuration through the EKiosk plugin system:

```json
{
  "qmlImportPaths": ["/custom/qml/modules"],
  "scriptEngine": "enabled",
  "graphicsBackend": "qml"
}
```

## Dependencies

### Qt Modules

- Core, Widgets, Quick, Qml
- WebEngineCore (for Qt6) or WebEngine (for Qt5)

### EKiosk SDK

- Plugin interfaces (`IPlugin`, `IPluginFactory`)
- GUI interfaces (`IGraphicsBackend`)
- Payment Processor interfaces (`ICore`, `Scripting::Core`)

## Building

The plugin is built using the EKiosk CMake system:

```cmake
ek_add_plugin(qml_backend
    FOLDER "plugins/GraphicBackends"
    SOURCES ${QMLBACKEND_SOURCES}
    QT_MODULES Core Widgets Quick Qml WebEngineCore
)
```

## Testing

Comprehensive test suite using mock kernel infrastructure:

```bash
# Build and run tests
cmake --build build/msvc --target qml_backend_test
./build/msvc/tests/plugins/QMLBackend/Debug/qml_backend_test.exe
```

Tests cover:

- Plugin loading and factory interface
- QML engine creation and script execution
- Graphics backend functionality
- Mock kernel integration

## Usage Examples

### Basic Plugin Usage

```cpp
// Plugin is loaded automatically by kernel
auto kernel = getKernel();
auto graphicsBackend = kernel->getInterface<SDK::GUI::IGraphicsBackend>();

// Create QML-based UI
auto mainWindow = graphicsBackend->createGraphicsItem("qml/MainWindow.qml");
```

### Script Execution

```cpp
// Execute payment scripts
auto scriptCore = qmlBackend->getScriptCore();
QVariant result = scriptCore->executePaymentScript(paymentData);
```

## File Structure

```text
QMLBackend/
├── CMakeLists.txt           # Build configuration
├── README.md               # This file
├── src/
│   ├── QMLBackendFactory.h/.cpp    # Plugin factory
│   ├── QMLBackend.h/.cpp          # Main plugin implementation
│   ├── QMLGraphicsItem.h/.cpp     # QML graphics wrapper
│   ├── Md5ValidatorQmlItem.h      # MD5 validation component
│   └── qml_backend.json           # Qt plugin metadata
└── tests/
    └── qml_backend_test.cpp       # Test suite
```

## Troubleshooting

### Common Issues

1. **QML Module Not Found**
   - Ensure QML import paths are configured
   - Check Qt QML installation

2. **Plugin Loading Failure**
   - Verify Qt plugin metadata (`qml_backend.json`)
   - Check plugin directory in build output

3. **Script Execution Errors**
   - Validate QML syntax
   - Ensure payment processor integration

## Development Notes

- Follows EKiosk plugin system architecture
- Uses Qt's meta-object system (Q_OBJECT, signals/slots)
- Implements proper resource management and cleanup
- Thread-safe design for kiosk environment

## See Also

- [Plugin System Architecture](../../../.github/copilot-instructions.md#plugin-system-architecture)
- [QMLBackend Documentation](../plugins/qmlbackend.md)
- [Testing Framework](../../../tests/plugins/common/)
