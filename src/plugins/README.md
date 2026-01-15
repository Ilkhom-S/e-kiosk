# Plugins

Shared library plugins for EKiosk.

## Quick Reference

| Category                              | Plugins            | Purpose              |
| ------------------------------------- | ------------------ | -------------------- |
| [Drivers](Drivers/)                   | Hardware drivers   | Device communication |
| [GraphicBackends](GraphicBackends/)   | UI renderers       | Screen display       |
| [ScenarioBackends](ScenarioBackends/) | Payment flows      | Business logic       |
| [Payments](Payments/)                 | Payment providers  | Payment processing   |
| [NativeScenarios](NativeScenarios/)   | Built-in scenarios | Utilities            |
| [NativeWidgets](NativeWidgets/)       | Built-in widgets   | UI components        |

## Building

```bash
# Build all plugins
cmake --build . --target all

# Build specific plugin category
cmake --build . --target bill_acceptors
cmake --build . --target native_backend
```

## Plugin Loading

Plugins are loaded at runtime by PluginsSDK:

```cpp
// Plugins are discovered from:
// - plugins/drivers/
// - plugins/graphics/
// - plugins/scenarios/
// - plugins/payments/

PluginsManager::instance()->scanPlugins();
```

## Creating a New Plugin

### 1. Create Directory

```text
plugins/<Category>/<PluginName>/
├── CMakeLists.txt
└── src/
    ├── Plugin.h
    ├── Plugin.cpp
    └── Factory.cpp
```

### 2. CMakeLists.txt

```cmake
include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB SOURCES src/*.cpp src/*.h)

# For all plugin types
ek_add_plugin(my_plugin
    SOURCES ${SOURCES}
    QT_MODULES Core SerialPort
    DEPENDS HardwareCommon DriversSDK
)
```

### 3. Export Factory

```cpp
#include <PluginsSDK/IPluginFactory.h>

class MyPluginFactory : public IPluginFactory {
    QString getPluginId() const override { return "my_plugin"; }
    QObject* createPlugin() override { return new MyPlugin(); }
};

extern "C" Q_DECL_EXPORT IPluginFactory* createPluginFactory() {
    return new MyPluginFactory();
}
```

## Platform Support

| Category         | Windows | Linux | macOS |
| ---------------- | ------- | ----- | ----- |
| Drivers          | ✅      | 🔬    | 🔬    |
| GraphicBackends  | ✅      | ✅    | ✅    |
| ScenarioBackends | ⚠️      | ⚠️    | ⚠️    |
| Payments         | ✅      | 🔬    | 🔬    |

⚠️ = Requires QtScript→QJSEngine migration for Qt6

## Documentation

See [docs/plugins/](../../../docs/plugins/) for detailed documentation.
