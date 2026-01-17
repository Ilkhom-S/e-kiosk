# Plugin Documentation Template

Use this template when creating documentation for new EKiosk plugins.

> **Reference Implementation**: See `src/plugins/TemplatePlugin/` for a minimal working example of a plugin implementation that can be used as a starting point for development. The template includes ready-to-copy files with `._h` and `._cpp` extensions containing comprehensive Russian documentation and examples.

## Title & Short Summary

[Plugin Name] - [One-line description of what the plugin does]

## Purpose

[Explain why this plugin exists and what problem it solves]

---

## Quick start 🔧

```cpp
// Minimal code example showing how to use the plugin
#include <[Plugin Interface Header]>

// Basic usage example
auto [pluginVariable] = [kernel/plugin]->[method call]();
```

---

## Features

- [Feature 1]
- [Feature 2]
- [Feature 3]

---

## Platform support

[Platform compatibility table - see examples below]

### Qt Version Compatibility

[Specify Qt version requirements and compatibility]

**Qt Version Support:**

- [ ] Qt5 compatible
- [ ] Qt6 compatible
- [ ] Version-specific requirements (explain why)

**Examples:**

- Qt5 + Qt6: Most plugins should support both
- Qt6 only: WebEngineBackend (uses Qt WebEngine)
- Qt5 only: WebKitBackend (uses deprecated Qt WebKit)

### Platform Support Table

- **Windows**: ✅ Full - [Platform-specific notes]
- **Linux**: ✅ Full - [Platform-specific notes]
- **macOS**: 🔬 TODO - [Platform-specific notes]

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

[Configuration options and examples]

---

## Usage / API highlights

[Key classes, methods, and usage examples]

---

## Integration

[CMake setup, plugin loading, dependencies]

---

## Testing

[Testing approach and commands]

---

## Dependencies

[List of dependencies]

---

## Troubleshooting

[Common issues and solutions]

---

## Migration notes

[Compatibility and migration information]

---

## Further reading

- [Plugin System Architecture](README.md#plugin-architecture)
- [Related documentation links]
