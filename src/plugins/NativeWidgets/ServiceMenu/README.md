# ServiceMenu Plugin

Service menu native widget plugin for EKiosk - provides comprehensive service and maintenance interface for kiosk devices.

## Purpose

The ServiceMenu plugin provides a complete service interface for kiosk maintenance, including:

- Device hardware testing and diagnostics
- Network and connection management
- Payment system configuration
- Key management and security
- System setup and configuration
- Logs and diagnostics
- Auto-encashment functionality

This plugin is essential for technical support, maintenance personnel, and administrators to manage and troubleshoot kiosk devices.

---

## Quick start 🔧

```cpp
// Access ServiceMenu plugin through the plugin system
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/IEnvironment.h>

// Get ServiceMenu plugin instance
SDK::Plugin::IPlugin *serviceMenu = environment->getPlugin("service_menu");
if (serviceMenu && serviceMenu->isReady()) {
    // Show service menu
    serviceMenu->show();
}
```

---

## Features

### Core Functionality

- **Main Service Window**: Central interface for accessing all service functions
- **Device Testing**: Comprehensive hardware device testing (bill acceptors, coin acceptors, dispensers, printers, etc.)
- **Network Management**: Dial-up and network connection configuration
- **Payment System**: Payment processing configuration and testing
- **Key Management**: Security key management and configuration
- **System Setup**: First-time setup wizard and system configuration
- **Diagnostics**: System diagnostics and logging
- **Auto-encashment**: Automated cash handling functionality

### Service Modules

- **Hardware Service**: Device status monitoring and testing
- **Network Service**: Connection management and testing
- **Payment Service**: Payment system configuration
- **Keys Service**: Security key management
- **Setup Service**: System setup and configuration
- **Token Service**: Token management
- **Logs Service**: System logging and diagnostics
- **Auto-encashment**: Automated cash handling

### User Interface Components

- **Main Service Window**: Primary navigation interface
- **Device Status Window**: Hardware device monitoring
- **Diagnostics Window**: System diagnostics
- **Virtual Keyboard**: On-screen keyboard input
- **Message Box**: Custom message dialogs
- **Wizard Framework**: Step-by-step setup wizards

---

## Platform support

### Qt Version Compatibility

**Qt Version Support:**

- ✅ Qt5 compatible
- ✅ Qt6 compatible
- No version-specific requirements

### Platform Support Table

- **Windows**: ✅ Full - Complete functionality including hardware device support
- **Linux**: ✅ Full - Complete functionality with platform-specific hardware support
- **macOS**: 🔬 TODO - Limited testing, hardware support may vary

---

## Configuration

The ServiceMenu plugin supports runtime configuration through the standard plugin configuration interface:

```cpp
// Get current configuration
QVariantMap config = serviceMenu->getConfiguration();

// Update configuration
config["showAdvancedOptions"] = true;
serviceMenu->setConfiguration(config);

// Save configuration
serviceMenu->saveConfiguration();
```

### Configuration Options

| Option                  | Type | Default | Description                           |
| ----------------------- | ---- | ------- | ------------------------------------- |
| `showAdvancedOptions`   | bool | false   | Show advanced service options         |
| `enableHardwareTests`   | bool | true    | Enable hardware testing functionality |
| `autoEncashmentEnabled` | bool | false   | Enable auto-encashment features       |
| `debugMode`             | bool | false   | Enable debug logging and features     |

---

## Usage / API highlights

### Main Service Menu Access

```cpp
// Show the main service menu
serviceMenu->show();

// Hide the service menu
serviceMenu->hide();

// Reset service menu state
serviceMenu->reset(QVariantMap());
```

### Device Testing

```cpp
// Access device testing functionality
auto backend = serviceMenu->getContext()["backend"].value<ServiceMenuBackend*>();
if (backend) {
    // Test bill acceptor
    backend->testBillAcceptor();

    // Test printer
    backend->testPrinter();

    // Get device status
    QVariantMap deviceStatus = backend->getDeviceStatus();
}
```

### Network Configuration

```cpp
// Configure network settings
QVariantMap networkConfig;
networkConfig["connectionType"] = "dialup";
networkConfig["phoneNumber"] = "+1234567890";
networkConfig["username"] = "admin";
networkConfig["password"] = "secure";

serviceMenu->notify("configureNetwork", networkConfig);
```

---

## Integration

### CMake Configuration

The ServiceMenu plugin is built using the standard EKiosk plugin CMake configuration:

```cmake
# From src/plugins/NativeWidgets/ServiceMenu/CMakeLists.txt
include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKTranslation.cmake)

ek_add_plugin(service_menu
    FOLDER "plugins"
    SOURCES ${SERVICEMENU_SOURCES} ${SERVICEMENU_UI_FILES} ${SERVICEMENU_RESOURCES}
    QT_MODULES Core Gui Widgets Network
    DEPENDS
        BasicApplication
        Connection
        PluginsSDK
        DriversSDK
        PPSDK
        NetworkTaskManager
        KeysUtils
        Log
        SysUtils
        ek_boost
    INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
    INSTALL_DIR plugins
)
```

### Build Dependencies

- **Core Modules**: BasicApplication, Connection, PluginsSDK
- **SDK Modules**: DriversSDK, PPSDK
- **Utility Modules**: NetworkTaskManager, KeysUtils, Log, SysUtils
- **External**: ek_boost

### Building the Plugin

```bash
# Configure and build the entire project (includes ServiceMenu)
cmake --preset win-msvc-qt5-x64
cmake --build build/win-msvc-qt5-x64 --target service_menu

# Build just the ServiceMenu plugin
cmake --build build/win-msvc-qt5-x64 --target service_menu
```

---

## Testing

The ServiceMenu plugin includes comprehensive tests using the EKiosk testing framework.

### Test Coverage

- Plugin loading and initialization
- Main service window functionality
- Device testing modules
- Network configuration
- Key management
- Setup wizard flow
- Auto-encashment functionality

### Running Tests

```bash
# Run ServiceMenu plugin tests
cmake --build build/win-msvc-qt5-x64 --target service_menu_test

# Run all plugin tests
cmake --build build/win-msvc-qt5-x64 --target run_tests
```

### Test Structure

```text
tests/plugins/
├── NativeWidgets/
│   └── ServiceMenu/
│       ├── service_menu_test.cpp
│       └── CMakeLists.txt
```

---

## Dependencies

### Internal Dependencies

- **BasicApplication**: Core application framework
- **Connection**: Network connection management
- **PluginsSDK**: Plugin system infrastructure
- **DriversSDK**: Hardware driver interfaces
- **PPSDK**: Payment processor interfaces
- **NetworkTaskManager**: Network task management
- **KeysUtils**: Cryptographic key utilities
- **Log**: Logging system
- **SysUtils**: System utilities

### External Dependencies

- **Qt**: Core, Gui, Widgets, Network modules
- **Boost**: Various Boost libraries (via ek_boost)

### Platform-Specific Dependencies

- **Windows**: Advapi32, Rasapi32 (for dial-up networking)

---

## Troubleshooting

### Common Issues

**Issue: Plugin fails to load**

- **Cause**: Missing dependencies or incorrect plugin path
- **Solution**: Verify all dependencies are built and plugin is in correct directory

**Issue: Device tests not working**

- **Cause**: Hardware not connected or drivers not installed
- **Solution**: Check hardware connections and install required drivers

**Issue: Network configuration fails**

- **Cause**: Invalid network settings or connection issues
- **Solution**: Verify network settings and check connection status

**Issue: UI elements not displaying correctly**

- **Cause**: Missing resource files or incorrect paths
- **Solution**: Verify resource files are included in build and paths are correct

### Debugging

Enable debug mode for detailed logging:

```cpp
QVariantMap config = serviceMenu->getConfiguration();
config["debugMode"] = true;
serviceMenu->setConfiguration(config);
```

Check logs for detailed error information:

```bash
# Check application logs
tail -f /var/log/ekiosk/service_menu.log

# Windows Event Viewer
# Look for EKiosk application logs
```

---

## Migration notes

### Version Compatibility

- **1.0**: Initial release (current version)
- **0.x**: Development versions (not recommended for production)

### API Stability

The ServiceMenu plugin API is considered stable. All public interfaces are documented and follow semantic versioning.

### Breaking Changes

No breaking changes in current version. Future major version updates will be documented with migration guides.

---

## Further reading

- [Plugin System Architecture](../docs/plugins/README.md#plugin-architecture)
- [ServiceMenu Plugin Documentation](../docs/plugins/servicemenu.md)
- [EKiosk Developer Guide](../docs/getting-started.md)
- [Testing Guide](../docs/testing.md)
