# QMLBackend Plugin

Qt QML-based graphics rendering backend for EKiosk applications.

## Purpose

The QMLBackend plugin provides Qt QML-based graphics rendering capabilities for EKiosk applications. It implements the `IGraphicsBackend` interface to enable QML-based user interfaces, supporting modern declarative UI development with Qt Quick.

---

## Quick start 🔧

```cpp
// Plugin is loaded automatically by the kernel
// Access through IGraphicsBackend interface

#include <SDK/GUI/IGraphicsBackend.h>

// Get graphics backend from kernel
auto kernel = getKernel();
auto graphicsBackend = kernel->getInterface<SDK::GUI::IGraphicsBackend>();

// Use QML rendering capabilities
graphicsBackend->createGraphicsItem("qml/MainWindow.qml");
```

---

## Features

- Qt QML and Qt Quick rendering engine
- Declarative UI development support
- Integration with EKiosk plugin system
- QML script execution and evaluation
- Graphics item creation and management
- Payment processor scripting integration

---

## Platform support

| Platform | Status  | Notes                  |
| -------- | ------- | ---------------------- |
| Windows  | ✅ Full | Qt QML fully supported |
| Linux    | ✅ Full | Qt QML fully supported |
| macOS    | 🔬 TODO | Qt QML support planned |

---

## Configuration

Configuration is managed through the plugin system:

```json
{
  "QMLBackend": {
    "qmlImportPaths": ["/path/to/qml/modules"],
    "scriptEngine": "enabled",
    "graphicsBackend": "qml"
  }
}
```

---

## Usage / API highlights

### Graphics Backend Interface

```cpp
// Create QML-based graphics items
QSharedPointer<SDK::GUI::IGraphicsItem> item =
    graphicsBackend->createGraphicsItem("qml/MainWindow.qml");

// Execute QML scripts
QVariant result = graphicsBackend->evaluateScript("1 + 2 * 3");

// Access QML engine
QQmlEngine *engine = graphicsBackend->getQmlEngine();
```

### Plugin Interface

```cpp
// Plugin lifecycle
bool initialized = qmlBackend->initialize(kernel);
bool started = qmlBackend->start();
bool stopped = qmlBackend->stop();
```

### Scripting Integration

```cpp
// Payment processor script execution
auto scriptCore = qmlBackend->getScriptCore();
scriptCore->executePaymentScript(paymentData);
```

---

## Integration

### CMake Dependencies

```cmake
ek_add_plugin(qml_backend
    FOLDER "plugins/GraphicBackends"
    SOURCES ${QMLBACKEND_SOURCES}
    QT_MODULES Core Widgets Quick Qml WebEngineCore
)
```

### Plugin Loading

The plugin is automatically discovered and loaded by the EKiosk kernel:

1. Qt plugin system scans plugin directories
2. QMLBackend factory creates plugin instances
3. Plugin receives kernel reference during initialization
4. Graphics backend becomes available through kernel interfaces

---

## Testing

Comprehensive tests using mock kernel infrastructure:

```cpp
#include "../common/PluginTestBase.h"

class QMLBackendTest : public QObject {
    Q_OBJECT

private slots:
    void testPluginLoading();
    void testFactoryInterface();
    void testQmlEngineCreation();
};
```

Run tests with:

```bash
cmake --build build/msvc --target qml_backend_test
```

---

## Dependencies

- Qt Core, Widgets, Quick, Qml modules
- EKiosk SDK (Plugin, GUI, PaymentProcessor interfaces)
- QML import paths for custom components

---

## Troubleshooting

### Common Issues

**QML Import Path Errors**

```
QQmlApplicationEngine failed to load component
qrc:/qml/MainWindow.qml:3 module "CustomModule" is not installed
```

**Solution**: Ensure QML import paths are configured in plugin settings

**Plugin Loading Failures**

```
Plugin "qml_backend" not loaded
```

**Solution**: Verify Qt QML modules are available and plugin metadata is correct

**Script Execution Errors**

```
QML script execution failed
```

**Solution**: Check script syntax and ensure payment processor integration is properly configured

---

## Migration notes

- **Qt5/Qt6 Compatibility**: Plugin supports both Qt5 and Qt6 with version-specific module selection
- **Legacy Migration**: Replaces older native rendering backends for modern QML-based UIs
- **API Stability**: Implements stable IGraphicsBackend interface for consistent integration

---

## Further reading

- [Plugin System Architecture](README.md#plugin-architecture)
- [QML Documentation](https://doc.qt.io/qt-5/qmlapplications.html)
- [EKiosk GUI Interfaces](../../../include/SDK/GUI/)
