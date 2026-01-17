# EKiosk Plugins Documentation

This directory contains comprehensive documentation for all EKiosk plugins. Each plugin has its own documentation file following the standard template.

## Plugin Architecture

EKiosk uses a modular plugin system based on Qt plugins with custom factory interfaces. All plugins must implement the `SDK::Plugin::IPluginFactory` interface and be built using the `ek_add_plugin()` CMake helper.

### Qt Version Compatibility

All plugins should be **Qt 5/6 agnostic** by default, supporting both Qt versions where possible. When Qt version-specific code is required, use proper version checks and document the reasons.

#### Qt Version Detection in CMake

```cmake
# Check Qt version and set appropriate modules
if(QT_VERSION_MAJOR EQUAL 5)
    set(QT_COMPONENTS Core Widgets Network)
elseif(QT_VERSION_MAJOR EQUAL 6)
    set(QT_COMPONENTS Core Widgets Network)
endif()

# Use version-agnostic syntax
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ${QT_COMPONENTS})
```

#### Qt Version Detection in Code

```cpp
// Use version macros for conditional compilation
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Qt6 specific code
    qDebug() << "Running on Qt6";
#else
    // Qt5 specific code
    qDebug() << "Running on Qt5";
#endif

// Or use runtime checks
if (QApplication::platformName() == "wayland" && QT_VERSION_MAJOR >= 6) {
    // Qt6 Wayland specific code
}
```

#### When Qt Version-Specific Plugins Are Needed

Some plugins may be Qt version-specific due to:

- **API Changes**: Qt6 introduced breaking changes in some modules
- **Deprecated Features**: Qt5 features removed in Qt6
- **Platform Support**: Different platform support between versions

**Examples:**

- **WebEngineBackend**: Qt6 only (Qt WebEngine)
- **WebKitBackend**: Qt5 only (Qt WebKit, deprecated in Qt6)

Document version requirements clearly in plugin README and docs.

### Platform Compatibility

Plugins should specify supported platforms and any limitations:

- **Windows**: ✅ Full support - All plugins supported
- **Linux**: ✅ Full support - All plugins supported
- **macOS**: 🔬 TODO - Limited testing

### Directory Layout

```text
src/plugins/
├── CategoryName/                    # Plugin category (e.g., GraphicBackends, Payments)
│   ├── PluginName/                  # Individual plugin
│   │   ├── CMakeLists.txt           # Plugin build configuration
│   │   ├── README.md                # Plugin-specific documentation
│   │   ├── src/                     # Source files
│   │   │   ├── PluginFactory.h/.cpp # Factory implementation
│   │   │   ├── PluginImpl.h/.cpp    # Plugin implementation
│   │   │   └── plugin.json          # Qt plugin metadata
│   │   └── tests/                   # Plugin-specific tests
│   │       └── plugin_test.cpp
```

### Core Interfaces

- **`SDK::Plugin::IPluginFactory`**: Creates plugin instances, provides metadata
- **`SDK::Plugin::IPlugin`**: Base plugin interface with lifecycle methods
- **`SDK::Plugin::IKernel`**: Application kernel providing services to plugins

### Plugin Loading and Lifecycle

1. **Discovery**: Qt plugin system scans plugin directories
2. **Registration**: Plugins register with REGISTER_PLUGIN_WITH_PARAMETERS macro
3. **Instantiation**: Factory creates plugin instances on demand
4. **Initialization**: Plugin receives kernel reference and initializes
5. **Operation**: Plugin runs and provides services
6. **Shutdown**: Plugin cleanup and resource release

### Best Practices

- **Error Handling**: Always check return values and handle failures gracefully
- **Logging**: Use kernel-provided logger for all diagnostic output
- **Thread Safety**: Document thread safety guarantees
- **Resource Management**: Properly clean up resources in stop()/destructor
- **Configuration**: Support runtime configuration changes
- **Testing**: Maintain high test coverage with mock kernel
- **Documentation**: Keep README and docs/plugins/ current

## Creating New Plugins

> **Note**: The `TemplatePlugin` in `src/plugins/TemplatePlugin/` serves as a minimal working example and template for creating new plugins. It demonstrates the basic plugin structure and can be used as a starting point for development.

### 1. Plugin Factory Implementation

```cpp
class MyPluginFactory : public QObject, public SDK::Plugin::IPluginFactory {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory" FILE "my_plugin.json")
    Q_INTERFACES(SDK::Plugin::IPluginFactory)

public:
    // IPluginFactory implementation
    QString getName() const override { return "My Plugin"; }
    QString getDescription() const override { return "Description of my plugin"; }
    QString getAuthor() const override { return "Author Name"; }
    QString getVersion() const override { return "1.0"; }
    QStringList getPluginList() const override { return QStringList() << "MyPlugin.Instance"; }

    SDK::Plugin::IPlugin *createPlugin(const QString &instancePath) override {
        return new MyPlugin(instancePath);
    }
};
```

### 2. Plugin Implementation

```cpp
class MyPlugin : public SDK::Plugin::IPlugin {
public:
    MyPlugin(const QString &instancePath) : m_instancePath(instancePath) {}

    bool initialize(SDK::Plugin::IKernel *kernel) override {
        m_kernel = kernel;
        m_log = kernel->getLog("MyPlugin");

        // Qt version compatibility example
        #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            qInfo() << "Running on Qt6";
        #else
            qInfo() << "Running on Qt5";
        #endif

        return true;
    }

    bool start() override {
        LOG(m_log, LogLevel::Normal, "MyPlugin started");
        return true;
    }

    bool stop() override {
        LOG(m_log, LogLevel::Normal, "MyPlugin stopped");
        return true;
    }

private:
    QString m_instancePath;
    SDK::Plugin::IKernel *m_kernel;
    ILog *m_log;
};
```

### 3. Qt Plugin Metadata (plugin.json)

```json
{
  "IID": "SDK.Plugin.PluginFactory",
  "version": "1.0",
  "name": "MyPlugin",
  "description": "My plugin description",
  "author": "Author Name"
}
```

### 4. CMake Configuration

```cmake
include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

# Qt version compatibility
if(QT_VERSION_MAJOR EQUAL 6)
    set(QT_COMPONENTS Core Widgets WebEngineCore)
elseif(QT_VERSION_MAJOR EQUAL 5)
    set(QT_COMPONENTS Core Widgets WebKit)
endif()

set(MY_PLUGIN_SOURCES
    src/MyPluginFactory.cpp
    src/MyPlugin.cpp
)

ek_add_plugin(my_plugin
    FOLDER "plugins/CategoryName"
    SOURCES ${MY_PLUGIN_SOURCES}
    QT_MODULES ${QT_COMPONENTS}  # Use version-appropriate modules
)
```

## Accessing Services

Plugins in EKiosk have access to various system services through the plugin environment and kernel. This allows plugins to integrate with core functionality like logging, cryptography, networking, and settings management.

### Key Services Available

- **Environment (IEnvironment)**: Provides access to plugin context and basic services
- **Logging (ILog)**: Centralized logging system for diagnostic output
- **Core Services (ICore)**: Main application services including cryptography, networking, and settings
- **Graphics Engine (IGraphicsEngine)**: Available in graphics backend plugins for rendering integration

### Service Access Patterns

```cpp
// Basic service access in plugin constructor/initialize
TemplatePlugin::TemplatePlugin(SDK::Plugin::IEnvironment *aEnvironment)
    : mEnvironment(aEnvironment) {
    // Get logger
    ILog *log = mEnvironment->getLog("TemplatePlugin");

    // Access core services
    auto core = dynamic_cast<SDK::PaymentProcessor::ICore*>(
        mEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

    // Use services...
}
```

### Error Handling

Always handle service unavailability gracefully:

```cpp
try {
    auto core = dynamic_cast<SDK::PaymentProcessor::ICore*>(
        mEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
    if (core) {
        // Use core services
        auto cryptService = core->getCryptService();
    }
} catch (const SDK::PaymentProcessor::ServiceIsNotImplemented &e) {
    LOG(mEnvironment->getLog("TemplatePlugin"), LogLevel::Error,
        QString("Service not available: %1").arg(e.what()));
}
```

> **For comprehensive service documentation**, see the [Services Documentation](../services/README.md) which provides detailed information about all available services, their features, usage patterns, and implementation examples.
> **For detailed examples and code snippets**, see the [Plugin Documentation Template](template.md#accessing-services) which includes comprehensive examples for accessing environment settings, logging, core services, graphics engine services, and error handling patterns.

## Plugin Testing Framework

All plugins must have comprehensive tests using the mock kernel infrastructure. Tests should achieve 100% coverage of all public methods in plugins and modules, including the complete call chain of classes and dependencies used during plugin execution.

### Coverage Requirements

**100% Coverage Definition:**

- Test all public methods of the plugin factory and implementation classes
- Test all classes instantiated and called by the plugin (e.g., AdPluginFactory → AdPluginImpl → dependent services)
- Cover error paths, edge cases, and integration scenarios
- Use DebugUtils for enhanced debugging when tests fail or for complex scenarios

### Test Structure

```text
tests/plugins/
├── common/                          # Shared testing utilities
│   ├── MockObjects.h/.cpp           # Mock implementations
│   ├── PluginTestBase.h/.cpp        # Base test class
│   └── CMakeLists.txt
├── CategoryName/
│   ├── PluginName/
│   │   ├── plugin_test.cpp          # Plugin-specific tests
│   │   └── CMakeLists.txt
```

### Writing Plugin Tests

```cpp
#include "../common/PluginTestBase.h"

class MyPluginTest : public QObject {
    Q_OBJECT

private slots:
    void testPluginLoading();
    void testPluginInitialization();
    void testPluginFunctionality();

private:
    PluginTestBase m_testBase;
};

void MyPluginTest::testPluginLoading() {
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);
    QCOMPARE(factory->getName(), QString("My Plugin"));
}

void MyPluginTest::testPluginInitialization() {
    // Test plugin initialization with mock kernel
    // Verify proper setup and error handling
}

void MyPluginTest::testPluginFunctionality() {
    // Test actual plugin functionality
    // Use mock objects to verify behavior
}
```

### Test CMake Configuration

```cmake
ek_add_test(my_plugin_test
    FOLDER "tests/plugins"
    SOURCES plugin_test.cpp
    DEPENDS PluginTestCommon my_plugin
    QT_MODULES Test Core
)
```

## Plugin Documentation Requirements

### Plugin README.md

Each plugin must have a README.md in its root directory with:

- Plugin purpose and functionality
- Configuration options
- Usage examples
- Dependencies and requirements
- Build instructions
- Testing instructions

### Central Plugin Documentation

Full documentation must be maintained in `docs/plugins/` with:

- Plugin architecture overview
- Integration guides
- API reference
- Configuration reference
- Troubleshooting guides
- Migration notes

### Documentation Checklist

- [ ] Plugin README.md exists and is up-to-date
- [ ] docs/plugins/PluginName.md exists with full documentation
- [ ] API documentation includes all public interfaces
- [ ] Configuration examples provided
- [ ] Integration examples for common use cases
- [ ] Testing documentation includes coverage and CI requirements

## Development Guidelines

- [Creating New Plugins](#creating-new-plugins)
- [Plugin Testing Framework](#plugin-testing-framework)
- [Plugin Documentation Requirements](#plugin-documentation-requirements)

## Testing

All plugins include comprehensive tests using the mock kernel infrastructure:

- Unit tests for plugin logic
- Integration tests with mock kernel
- Factory interface validation
- Lifecycle testing

Run plugin tests with:

```bash
cmake --build build/msvc --target <plugin>_test
```

## Available Plugins

The following plugins are installed by default with EKiosk:

> **Note**: `TemplatePlugin` is a development template, not a production plugin. See [Creating New Plugins](#creating-new-plugins) for how to use it as a starting point for new plugin development.

### Graphic Backends

- **[QMLBackend](qmlbackend.md)**: Qt QML-based graphics rendering
- **NativeBackend**: Native platform rendering
- **[WebEngineBackend](webenginebackend.md)**: Chromium-based rendering (Qt6 only)
- **[WebKitBackend](webkitbackend.md)**: Legacy WebKit rendering (Qt5 only)

### Payment Systems

- **Humo**: Payment processing for Humo cards

### Hardware Drivers

- **BillAcceptor**: Bill acceptor device support
- **BillDispensers**: Bill dispenser device support
- **CardReader**: Card reader device support
- **CoinAcceptor**: Coin acceptor device support
- **FR**: Fiscal registration device support
- **HID**: Human Interface Device support
- **IOPort**: I/O port device support
- **Modem**: Modem device support
- **Parameters**: Device parameter management
- **Printer**: Printer device support
- **VirtualDevices**: Virtual device simulation
- **Watchdog**: Watchdog timer device support

### Business Logic

- **Migrator3000**: Database migration utilities
- **ScreenMaker**: Screen creation and management
- **UCS**: UCS scenario backend
- **Uniteller**: Uniteller payment scenario backend

### User Interface

- **ServiceMenu**: Service menu interface
- **Ad**: Advertising display components

## Contributing

When adding new plugins:

1. Create plugin in appropriate category under `src/plugins/`
2. Add plugin documentation to this directory
3. Update plugin README.md in plugin root
4. Add comprehensive tests using `PluginTestBase`
5. Update this index file

## See Also

- [Module Documentation](../modules/): Core application modules
- [Testing Guide](../testing.md): General testing practices
