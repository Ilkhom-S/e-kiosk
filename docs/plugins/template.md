# Plugin Documentation Template

Comprehensive template for creating documentation for new EKiosk plugins.

> **Reference Implementation**: See `src/plugins/TemplatePlugin/` for a minimal working example of a plugin implementation that can be used as a starting point for development. The template includes ready-to-copy files with `._h` and `._cpp` extensions containing comprehensive Russian documentation and examples.

## Title & Short Summary

**[Plugin Name]** - **[One-line description of what the plugin does]**

## Purpose

**[Explain why this plugin exists and what problem it solves]**

This plugin provides **[detailed description of functionality and benefits]**.

---

## Quick start 🔧

```cpp
// Minimal code example showing how to use the plugin
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/IEnvironment.h>

// Get plugin instance
SDK::Plugin::IPlugin *plugin = environment->getPlugin("[plugin_name]");
if (plugin && plugin->isReady()) {
    // Basic usage example
    auto service = environment->getInterface<[ServiceInterface]>("[ServiceName]");
    if (service) {
        // Example usage
        service->doSomething();
    }
}
```

---

## Features

### Core Functionality

- **[Feature 1]**: [Detailed description]
- **[Feature 2]**: [Detailed description]
- **[Feature 3]**: [Detailed description]

### Advanced Features

- **[Advanced Feature 1]**: [Detailed description]
- **[Advanced Feature 2]**: [Detailed description]

### Integration Points

- **[Integration Point 1]**: [Description]
- **[Integration Point 2]**: [Description]

---

## Platform support

### Qt Version Compatibility

**Qt Version Support:**

- ✅ Qt5 compatible
- ✅ Qt6 compatible
- ❌ Version-specific requirements (explain why)

**Examples:**

- **Qt5 + Qt6**: Most plugins should support both
- **Qt 5.6+ and Qt6**: WebEngineBackend (uses Qt WebEngine)
- **Qt 5.0-5.5 only (deprecated)**: WebKitBackend (uses deprecated Qt WebKit)

### Platform Support Table

- **Windows**: ✅ Full - [Platform-specific notes]
  - [Feature support details]
  - [Platform-specific requirements]
  - [Known limitations]

- **Linux**: ✅ Full - [Platform-specific notes]
  - [Feature support details]
  - [Platform-specific requirements]
  - [Known limitations]

- **macOS**: 🔬 TODO - [Platform-specific notes]
  - [Feature support details]
  - [Platform-specific requirements]
  - [Known limitations]

---

## Accessing Services

Plugins in EKiosk have access to various system services through the plugin environment. Here's how to access common services:

### Environment & Logging

```cpp
// Constructor receives IEnvironment
TemplatePlugin::TemplatePlugin(SDK::Plugin::IEnvironment *aEnvironment, const QString &aInstancePath)
    : mEnvironment(aEnvironment), mInstancePath(aInstancePath) {
}

// Get logger for this plugin
ILog *log = mEnvironment->getLog("TemplatePlugin");
LOG(log, LogLevel::Normal, "Plugin initialized");

// For graphics plugins, logging is available via the graphics engine
// (passed during initialize() call)
ILog *log = mEngine->getLog();
```

### Core Services

```cpp
// Access the main payment processor core
SDK::PaymentProcessor::ICore *core = dynamic_cast<SDK::PaymentProcessor::ICore*>(
    mEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

// For graphics plugins
SDK::PaymentProcessor::ICore *core = mEngine->getGraphicsHost()->getInterface<SDK::PaymentProcessor::ICore>(
    SDK::PaymentProcessor::CInterfaces::ICore);
```

### Common Services via Core

```cpp
// Cryptography service
auto cryptService = core->getCryptService();
ICryptEngine *cryptEngine = cryptService->getCryptEngine();

// Network service
auto networkService = core->getNetworkService();
NetworkTaskManager *network = networkService->getNetworkTaskManager();

// Settings service
auto settingsService = core->getSettingsService();
// Access terminal settings
SDK::PaymentProcessor::TerminalSettings *terminalSettings = settingsService->getAdapter()->getTerminalSettings();
```

### Graphics Engine Services (Graphics Plugins)

```cpp
// In initialize() method for graphics backends
bool TemplateGraphicsBackend::initialize(SDK::GUI::IGraphicsEngine *aEngine) {
    mEngine = aEngine;

    // Access graphics host for additional services
    auto graphicsHost = mEngine->getGraphicsHost();

    // Get core via graphics host
    auto core = graphicsHost->getInterface<SDK::PaymentProcessor::ICore>(
        SDK::PaymentProcessor::CInterfaces::ICore);

    // Access other registered interfaces
    foreach (auto interfaceName, graphicsHost->getInterfacesName()) {
        auto service = graphicsHost->getInterface<QObject>(interfaceName);
        // Use service...
    }

    return true;
}
```

### Service Availability

- **Environment**: Always available (passed in constructor)
- **Core Services**: Available after plugin initialization
- **Graphics Engine**: Only available in graphics backend plugins
- **Logging**: Always available through environment or graphics engine

### Error Handling

```cpp
try {
    auto core = dynamic_cast<SDK::PaymentProcessor::ICore*>(
        mEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));
    // Use core...
} catch (const SDK::PaymentProcessor::ServiceIsNotImplemented &e) {
    LOG(mEnvironment->getLog("TemplatePlugin"), LogLevel::Error,
        QString("Service not available: %1").arg(e.what()));
}
```

---

## Configuration

### Plugin Configuration

The plugin supports comprehensive runtime configuration:

```cpp
// Get current configuration
QVariantMap config = plugin->getConfiguration();

// Update configuration options
config["option1"] = value1;
config["option2"] = value2;

// Apply configuration
plugin->setConfiguration(config);

// Save configuration permanently
plugin->saveConfiguration();
```

### Configuration Options Reference

| Option    | Type | Default | Description | Example Values     |
| --------- | ---- | ------- | ----------- | ------------------ |
| `option1` | type | default | Description | "value1", "value2" |
| `option2` | type | default | Description | true, false        |

### Configuration File Format

Configuration is stored in INI format:

```ini
[PluginName]
; Section 1
option1=value1
option2=value2

; Section 2
option3=value3
option4=value4
```

---

## Usage / API highlights

### Main Plugin Operations

```cpp
// Plugin lifecycle management
bool initialized = plugin->initialize();
bool started = plugin->start();
bool stopped = plugin->stop();

// Plugin state management
plugin->show();
plugin->hide();
plugin->reset(QVariantMap());

// Plugin status
bool isReady = plugin->isReady();
QString error = plugin->getError();
QVariantMap context = plugin->getContext();
```

### Service Interface Usage

```cpp
// Access main service interface
auto service = environment->getInterface<ServiceInterface>("ServiceName");
if (service) {
    // Use service methods
    service->doSomething();
    service->doSomethingElse();
}
```

### Advanced Usage

```cpp
// Complex operations
QVariantMap parameters;
parameters["param1"] = value1;
parameters["param2"] = value2;

QVariant result = service->complexOperation(parameters);
```

---

## Integration

### CMake Configuration

```cmake
# Plugin source files
set(PLUGIN_SOURCES
    src/PluginFactory.cpp
    src/PluginFactory.h
    src/PluginImpl.cpp
    src/PluginImpl.h
    # Add other source files...
)

# Plugin definition
ek_add_plugin(plugin_name
    FOLDER "plugins/CategoryName"

    # Source files
    SOURCES ${PLUGIN_SOURCES}

    # Required Qt modules
    QT_MODULES
        Core      # Core Qt functionality
        # Add other required Qt modules...

    # Internal dependencies
    DEPENDS
        BasicApplication    # Core application framework
        PluginsSDK          # Plugin system infrastructure
        # Add other dependencies...

    # Include directories
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src

    # Installation directory
    INSTALL_DIR
        plugins
)
```

### Build Process

```bash
# Configure the project
cmake --preset [preset_name]

# Build the plugin specifically
cmake --build build/[build_dir] --target plugin_name

# Build with verbose output
cmake --build build/[build_dir] --target plugin_name --verbose
```

### Plugin Loading Sequence

1. **Discovery**: Qt plugin system scans plugin directories
2. **Registration**: Plugin registers with EKiosk plugin system
3. **Instantiation**: Plugin factory creates plugin instances
4. **Initialization**: Plugin receives environment and initializes
5. **Service Registration**: Plugin services register with kernel
6. **Operation**: Plugin becomes available for use

---

## Testing

### Test Framework Architecture

```
tests/plugins/PluginName/
├── plugin_test.cpp          # Main test suite
├── mock_services.h/.cpp     # Mock service implementations
├── test_data/               # Test data files
└── CMakeLists.txt           # Test build configuration
```

### Test Coverage Matrix

| Component          | Unit Tests | Integration Tests | Error Paths | Performance Tests |
| ------------------ | ---------- | ----------------- | ----------- | ----------------- |
| Plugin Loading     | ✅         | ✅                | ✅          | ❌                |
| Configuration      | ✅         | ✅                | ✅          | ❌                |
| Core Functionality | ✅         | ✅                | ✅          | ✅                |
| Error Handling     | ✅         | ✅                | ✅          | ❌                |

### Test Implementation Examples

```cpp
#include "../common/PluginTestBase.h"

class PluginTest : public QObject {
    Q_OBJECT

private slots:
    void testPluginLoading();
    void testPluginInitialization();
    void testCoreFunctionality();
    void testErrorHandling();
};

void PluginTest::testPluginLoading() {
    // Test plugin factory and loading
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory("plugin_name");
    QVERIFY(factory != nullptr);
    QCOMPARE(factory->getName(), QString("Plugin Name"));
}

void PluginTest::testCoreFunctionality() {
    // Test plugin core functionality
    auto plugin = m_testBase.createPlugin("plugin_name");
    QVERIFY(plugin->isReady());

    // Test plugin operations
    // ...
}
```

### Test Execution

```bash
# Run plugin tests specifically
cmake --build build/[build_dir] --target plugin_test

# Run tests with detailed output
ctest --output-on-failure --verbose -R plugin

# Generate test coverage report
cmake --build build/[build_dir] --target coverage
```

---

## Dependencies

### Internal Dependencies

| Dependency                 | Purpose                      | Version Requirements | Criticality |
| -------------------------- | ---------------------------- | -------------------- | ----------- |
| **BasicApplication**       | Core application framework   | 1.0+                 | ✅ Critical |
| **PluginsSDK**             | Plugin system infrastructure | 1.0+                 | ✅ Critical |
| **Add other dependencies** | Description                  | 1.0+                 | ✅ Critical |

### External Dependencies

| Dependency                 | Purpose               | Version Requirements | Platform Notes |
| -------------------------- | --------------------- | -------------------- | -------------- |
| **Qt Core**                | Core Qt functionality | Qt5/Qt6              | Cross-platform |
| **Add other dependencies** | Description           | Qt5/Qt6              | Cross-platform |

---

## Troubleshooting

### Common Issues and Solutions

#### Plugin Loading Issues

**Symptom**: Plugin fails to load
**Causes**: Missing dependencies, configuration errors
**Solutions**: Verify dependencies, check configuration

#### Service Integration Issues

**Symptom**: Services not accessible
**Causes**: Plugin not initialized, service unavailable
**Solutions**: Check plugin status, verify service availability

---

## Migration notes

### Version Compatibility

| Version | Notes                  |
| ------- | ---------------------- |
| 1.0     | Current stable release |
| 0.9     | Beta release           |
| 0.1-0.8 | Development versions   |

### Migration Paths

**From 0.x to 1.0**: No breaking changes, recommended upgrade

---

## Further reading

- [Plugin System Architecture](README.md#plugin-architecture)
- [EKiosk Developer Guide](../../docs/getting-started.md)
- [Testing Guide](../../docs/testing.md)
- [CMake Build System](../../docs/build-guide.md)

### Related Plugins

- **[Related Plugin 1](plugin1.md)**: Description
- **[Related Plugin 2](plugin2.md)**: Description

### External Resources

- [Qt Documentation](https://doc.qt.io/)
- [CMake Documentation](https://cmake.org/documentation/)
- [Other relevant resources](https://example.com)

---

## Configuration Reference

### Complete Configuration Specification

```ini
[PluginName]
; Section 1
option1=default_value
option2=default_value

; Section 2
option3=default_value
option4=default_value
```

---

## Best Practices

### Development Best Practices

- Follow EKiosk coding standards
- Write comprehensive unit tests
- Document all public APIs
- Use meaningful variable names

### Usage Best Practices

- Always check plugin readiness
- Handle errors gracefully
- Validate configuration changes
- Monitor resource usage

### Deployment Best Practices

- Provide default configurations
- Document all dependencies
- Test on target platforms
- Monitor performance metrics

---

## Support

### Support Resources

- **GitHub Issues**: Report bugs and feature requests
- **Documentation**: Check for updates
- **Community Forum**: Join discussions
- **Professional Support**: Contact support team

### Support Request Template

```markdown
## Support Request

**Plugin**: [Plugin Name]
**Version**: [Version]
**Platform**: [Platform]
**Qt Version**: [Qt Version]

### Issue Description

[Description]

### Steps to Reproduce

1. [Step 1]
2. [Step 2]

### Expected Behavior

[Expected behavior]

### Actual Behavior

[Actual behavior]

### Environment

- OS: [OS Version]
- Hardware: [Details]
- Dependencies: [List]
```

---

## License

### License Information

The plugin is licensed under the **EKiosk Commercial License**.

### Third-Party Licenses

| Component | License | Usage              |
| --------- | ------- | ------------------ |
| Qt        | LGPL    | Core functionality |
| Other     | License | Description        |

---

## Appendix

### Glossary

| Term | Definition |
| ---- | ---------- |
| Term | Definition |

### Acronyms

| Acronym | Expansion                         |
| ------- | --------------------------------- |
| API     | Application Programming Interface |

### References

- [Reference 1](https://example.com)
- [Reference 2](https://example.com)

### Change Log

**Version 1.0**: Initial release

### Contributors

- Development Team
- QA Team
- Documentation Team

### Acknowledgements

Special thanks to contributors and testers.
