# TemplatePlugin

A minimal template plugin for EKiosk that serves as a starting point for creating new plugins.

## Purpose

This plugin provides the basic structure and boilerplate code needed to create a new EKiosk plugin. It implements the minimal required interfaces and demonstrates best practices for plugin development.

## Quick Start

1. **Copy the template files:**

   ```bash
   cp -r src/plugins/TemplatePlugin src/plugins/YourPluginName
   cd src/plugins/YourPluginName
   ```

2. **Copy the template source files:**

   ```bash
   cp src/TemplatePlugin._h src/YourPlugin.h
   cp src/TemplatePlugin._cpp src/YourPlugin.cpp
   cp src/TemplatePluginFactory._h src/YourPluginFactory.h
   cp src/TemplatePluginFactory._cpp src/YourPluginFactory.cpp
   ```

3. **Rename and customize:**
   - Use search/replace to change `TemplatePlugin` → `YourPlugin`
   - Update class names, constants, and metadata
   - Add your plugin-specific functionality
   - Update `CMakeLists.txt` and `template_plugin.json`

4. **Implement your functionality:**
   - Add business logic to the plugin class methods
   - Implement required interfaces (IGraphicsBackend, etc.)
   - Add proper error handling and logging
   - Update tests to verify your functionality

## File Structure

```text
TemplatePlugin/
├── CMakeLists.txt           # Build configuration
├── README.md                # This documentation
├── src/                     # Source files
│   ├── TemplatePluginFactory.h/cpp  # Main plugin factory implementation
│   ├── TemplatePlugin.h/cpp         # Example plugin implementation
│   ├── TemplatePlugin._h            # Header template (copy this!)
│   ├── TemplatePlugin._cpp          # Implementation template (copy this!)
│   ├── TemplatePluginFactory._h     # Factory header template (copy this!)
│   ├── TemplatePluginFactory._cpp   # Factory implementation template (copy this!)
│   └── template_plugin.json         # Qt plugin metadata
└── ../tests/plugins/TemplatePlugin/  # Unit tests (separate directory)
    ├── CMakeLists.txt       # Test build configuration
    └── test_plugin_test.cpp # Comprehensive plugin tests
```

## Using the Templates

The template files with `._h` and `._cpp` extensions are ready-to-use templates for creating new plugins:

### 1. Copy Template Files

```bash
# Copy header and implementation templates
cp src/TemplatePlugin._h src/YourPlugin.h
cp src/TemplatePlugin._cpp src/YourPlugin.cpp
cp src/TemplatePluginFactory._h src/YourPluginFactory.h
cp src/TemplatePluginFactory._cpp src/YourPluginFactory.cpp
```

### 2. Rename and Customize

- Replace all occurrences of `TemplatePlugin` with `YourPlugin`
- Update class names, namespaces, and constants
- Add your specific plugin logic
- Update metadata in the JSON file
- Modify CMakeLists.txt

### 3. Template File Contents

The template files include:

- **Comprehensive Russian comments** explaining each method and parameter
- **Error handling examples** with try-catch blocks
- **Resource management patterns** for proper cleanup
- **Configuration handling** with validation examples
- **Logging integration** with the plugin environment
- **Qt integration** with QObject inheritance
- **Plugin lifecycle management** (creation/destruction)

## Key Components

### Plugin Factory

- Inherits from `SDK::Plugin::PluginFactory`
- Implements `IPluginFactory` interface
- Provides plugin metadata (name, description, version)
- Overrides `createPlugin()` and `destroyPlugin()` for custom plugin instantiation
- Uses modern Qt5/6 `Q_PLUGIN_METADATA` instead of deprecated `Q_EXPORT_PLUGIN2`

### Plugin Implementation

- Implements `SDK::Plugin::IPlugin` interface
- Inherits from `QObject` for Qt integration
- Provides example "Hello World" functionality via `getHelloMessage()`
- Demonstrates configuration management and logging
- Shows proper resource cleanup in destructor

### Qt Plugin Metadata

- JSON file defining plugin properties
- Referenced by `Q_PLUGIN_METADATA` macro in factory class
- Contains IID, version, name, description, author

### CMake Configuration

- Uses `ek_add_plugin()` helper macro
- Defines plugin name, sources, dependencies
- Sets up proper folder structure in IDE

## Customization Guide

### 1. Update Plugin Identity

```cpp
// In TemplatePluginFactory.cpp
QString SDK::Plugin::PluginFactory::mName = "Your Plugin Name";
QString SDK::Plugin::PluginFactory::mDescription = "Description of your plugin";
QString SDK::Plugin::PluginFactory::mAuthor = "Your Name";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
QString SDK::Plugin::PluginFactory::mModuleName = "your_plugin_name";
```

### 2. Update JSON Metadata

```json
{
  "IID": "SDK.Plugin.PluginFactory",
  "version": "1.0",
  "name": "your_plugin_name",
  "description": "Description of your plugin",
  "author": "Your Name"
}
```

### 3. Update CMakeLists.txt

```cmake
ek_add_plugin(your_plugin_name
    FOLDER "plugins/YourCategory"
    SOURCES ${YOUR_PLUGIN_SOURCES}
    QT_MODULES Core  # Add required Qt modules
    DEPENDS PluginsSDK
)
```

### 4. Implement Plugin Logic

Override `createPlugin()` in your factory to return your custom plugin:

```cpp
SDK::Plugin::IPlugin *YourPluginFactory::createPlugin(const QString &aInstancePath, const QString &aConfigPath) {
    return new YourPlugin(this, aInstancePath);
}
```

Implement your plugin class inheriting from `SDK::Plugin::IPlugin`:

```cpp
class YourPlugin : public QObject, public SDK::Plugin::IPlugin {
    Q_OBJECT

public:
    YourPlugin(SDK::Plugin::IEnvironment *aEnvironment, const QString &aInstancePath);
    ~YourPlugin();

    // IPlugin interface
    QString getPluginName() const override;
    QString getConfigurationName() const override;
    QVariantMap getConfiguration() const override;
    void setConfiguration(const QVariantMap &aConfiguration) override;
    bool saveConfiguration() override;
    bool isReady() const override;

    // Your custom methods
    void doSomething();
};
```

### 5. Update Tests

Modify the test file to verify your plugin's functionality:

```cpp
void YourPluginTest::testPluginFunctionality() {
    // Load factory
    // Create plugin instance
    // Test your custom methods
    // Verify configuration
    // Clean up
}
```

## Testing

The template includes comprehensive unit tests that verify:

- Plugin library can be loaded dynamically
- Factory interface is accessible and properly configured
- Plugin instances can be created and destroyed safely
- Plugin functionality works correctly through the `IPlugin` interface
- Configuration management (set/get/save) operates properly
- Resource cleanup is handled correctly

Run tests with:

```bash
cmake --build build/msvc --target template_plugin_test
```

### Test Coverage

The tests demonstrate how to:

1. **Load the plugin factory** using Qt's dynamic plugin loading system
2. **Create plugin instances** via the factory's `createPlugin()` method
3. **Test plugin interfaces** using the standard `IPlugin` methods
4. **Verify configuration handling** (setConfiguration, getConfiguration, saveConfiguration)
5. **Test lifecycle management** (creation and destruction)
6. **Clean up resources** properly using `destroyPlugin()`

### Advanced Testing

For testing custom plugin functionality beyond the `IPlugin` interface:

- Use `qobject_cast<>()` to cast to the specific plugin type if needed
- Implement mock kernel environments using `PluginTestBase.h`
- Test with different configuration scenarios
- Verify logging and error handling

The template shows a complete working example that can be extended for more complex testing scenarios.

## Qt Version Compatibility

This template is designed to be **Qt 5/6 agnostic** and should work with both versions. The CMake configuration automatically detects the Qt version and sets appropriate dependencies.

### Qt Version-Specific Code Examples

If your plugin needs Qt version-specific code, use these patterns:

```cpp
// Version check in code
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Qt6 specific code
    qDebug() << "Qt6 detected";
#else
    // Qt5 specific code
    qDebug() << "Qt5 detected";
#endif
```

```cmake
# Version check in CMake
if(QT_VERSION_MAJOR EQUAL 6)
    # Qt6 specific configuration
    set(QT_COMPONENTS Core Widgets)
elseif(QT_VERSION_MAJOR EQUAL 5)
    # Qt5 specific configuration
    set(QT_COMPONENTS Core Widgets)
endif()
```

### When Version-Specific Plugins Are Needed

If your plugin can only support one Qt version, document it clearly:

- **Qt6 Only**: Use Qt WebEngine (WebEngineBackend)
- **Qt5 Only**: Use Qt WebKit (WebKitBackend)

## Platform Compatibility

- **Windows**: ✅ Full support
- **Linux**: ✅ Full support
- **macOS**: 🔬 TODO - Limited testing

## Documentation

When creating a new plugin from this template:

1. Create documentation in `docs/plugins/your_plugin.md`
2. Follow the template in `docs/plugins/template.md`
3. Update `docs/plugins/README.md` to include your plugin
4. Add your plugin to the appropriate category

## See Also

- [Plugin System Documentation](../../../docs/plugins/README.md)
- [Creating New Plugins](../../../docs/plugins/README.md#creating-new-plugins)
- [Plugin Template Documentation](../../../docs/plugins/template.md)
- [EKiosk Plugin Architecture](../../../docs/plugins/README.md#plugin-architecture)
