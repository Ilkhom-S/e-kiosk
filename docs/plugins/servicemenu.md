# ServiceMenu Plugin Documentation

Comprehensive documentation for the ServiceMenu native widget plugin.

## Title & Short Summary

**ServiceMenu** - Native widget plugin providing comprehensive service and maintenance interface for EKiosk devices.

## Purpose

The ServiceMenu plugin serves as the primary interface for kiosk maintenance, technical support, and administrative functions. It provides a centralized location for:

- **Device Management**: Testing, monitoring, and configuring hardware devices
- **System Configuration**: Network setup, payment system configuration, and security settings
- **Diagnostics**: System health monitoring, logging, and troubleshooting
- **Maintenance**: Auto-encashment, token management, and system updates

This plugin is essential for technical personnel to maintain and troubleshoot kiosk devices in production environments.

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

    // Configure service menu
    QVariantMap config;
    config["showAdvancedOptions"] = true;
    serviceMenu->setConfiguration(config);
}
```

---

## Features

### Core Service Modules

- **Hardware Service**: Comprehensive device testing and monitoring
  - Bill acceptor testing and configuration
  - Coin acceptor testing and configuration
  - Dispenser testing and configuration
  - Printer testing and configuration
  - HID device testing
  - Generic device testing framework

- **Network Service**: Connection management and testing
  - Dial-up connection configuration
  - Network connection testing
  - Connection status monitoring
  - Network troubleshooting tools

- **Payment Service**: Payment system configuration
  - Payment processor configuration
  - Transaction testing
  - Payment device management
  - Payment system diagnostics

- **Keys Service**: Security key management
  - Key generation and import
  - Key storage and management
  - Security configuration
  - Key-based authentication

- **Setup Service**: System configuration
  - First-time setup wizard
  - System parameter configuration
  - Device configuration
  - Network setup

- **Token Service**: Token management
  - Token generation and management
  - Token-based authentication
  - Token configuration

- **Logs Service**: System diagnostics
  - System log viewing
  - Diagnostic information
  - Error reporting
  - Log export functionality

- **Auto-encashment**: Automated cash handling
  - Cash collection automation
  - Encashment history
  - Encashment configuration
  - Cash handling reports

### User Interface Components

- **Main Service Window**: Primary navigation interface
- **Device Status Window**: Real-time hardware monitoring
- **Diagnostics Window**: System health and diagnostics
- **Virtual Keyboard**: On-screen input for touch devices
- **Message Box**: Custom dialog system
- **Wizard Framework**: Step-by-step configuration wizards

### Backend Services

- **HardwareManager**: Device management and testing
- **NetworkManager**: Network configuration and testing
- **PaymentManager**: Payment system management
- **KeysManager**: Security key management
- **ServiceMenuBackend**: Core backend functionality

---

## Platform support

### Qt Version Compatibility

**Qt Version Support:**

- ✅ Qt5 compatible
- ✅ Qt6 compatible
- No version-specific requirements

The ServiceMenu plugin is designed to work with both Qt5 and Qt6 without requiring version-specific code.

### Platform Support Table

- **Windows**: ✅ Full - Complete functionality including hardware device support
  - Full hardware device testing support
  - Dial-up networking support
  - Complete UI functionality
  - Native Windows integration

- **Linux**: ✅ Full - Complete functionality with platform-specific hardware support
  - Hardware device support through Linux drivers
  - Network configuration support
  - Complete UI functionality
  - Platform-specific device handling

- **macOS**: 🔬 TODO - Limited testing, hardware support may vary
  - Basic functionality tested
  - Hardware support depends on available drivers
  - UI functionality should work fully
  - Requires additional testing for production use

---

## Accessing Services

The ServiceMenu plugin provides access to various system services through the plugin environment.

### Environment & Logging

```cpp
// Constructor receives IEnvironment
ServiceMenu::ServiceMenu(SDK::Plugin::IEnvironment *aEnvironment, const QString &aInstancePath)
    : mEnvironment(aEnvironment), mInstancePath(aInstancePath) {
}

// Get logger for this plugin
ILog *log = mEnvironment->getLog("ServiceMenu");
LOG(log, LogLevel::Normal, "ServiceMenu plugin initialized");
```

### Core Services

```cpp
// Access the main payment processor core
SDK::PaymentProcessor::ICore *core = dynamic_cast<SDK::PaymentProcessor::ICore*>(
    mEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

// Access cryptography service
auto cryptService = core->getCryptService();
ICryptEngine *cryptEngine = cryptService->getCryptEngine();

// Access network service
auto networkService = core->getNetworkService();
NetworkTaskManager *network = networkService->getNetworkTaskManager();
```

### Plugin-Specific Services

```cpp
// Access ServiceMenu backend services
auto backend = mBackend.data();
if (backend) {
    // Hardware testing
    backend->testBillAcceptor();
    backend->testPrinter();

    // Network testing
    backend->testNetworkConnection();

    // Get system status
    QVariantMap status = backend->getSystemStatus();
}
```

---

## Configuration

### Plugin Configuration

The ServiceMenu plugin supports comprehensive runtime configuration:

```cpp
// Get current configuration
QVariantMap config = serviceMenu->getConfiguration();

// Update configuration options
config["showAdvancedOptions"] = true;
config["enableHardwareTests"] = true;
config["autoEncashmentEnabled"] = false;
config["debugMode"] = true;

// Apply configuration
serviceMenu->setConfiguration(config);

// Save configuration permanently
serviceMenu->saveConfiguration();
```

### Configuration Options Reference

| Option                  | Type   | Default    | Description                                       | Example Values                                            |
| ----------------------- | ------ | ---------- | ------------------------------------------------- | --------------------------------------------------------- |
| `showAdvancedOptions`   | bool   | false      | Show advanced service options and debugging tools | true, false                                               |
| `enableHardwareTests`   | bool   | true       | Enable hardware device testing functionality      | true, false                                               |
| `autoEncashmentEnabled` | bool   | false      | Enable auto-encashment features and UI            | true, false                                               |
| `debugMode`             | bool   | false      | Enable debug logging and diagnostic features      | true, false                                               |
| `defaultServiceTab`     | string | "hardware" | Default service tab to show on startup            | "hardware", "network", "payment", "keys", "setup", "logs" |
| `showDeviceStatus`      | bool   | true       | Show device status monitoring in main window      | true, false                                               |
| `enableNetworkTesting`  | bool   | true       | Enable network connection testing tools           | true, false                                               |

### Configuration File

Configuration is stored in the plugin's configuration file:

```
[ServiceMenu]
showAdvancedOptions=false
enableHardwareTests=true
autoEncashmentEnabled=false
debugMode=false
defaultServiceTab=hardware
```

---

## Usage / API highlights

### Main Service Menu Operations

```cpp
// Show the service menu
serviceMenu->show();

// Hide the service menu
serviceMenu->hide();

// Reset service menu to default state
serviceMenu->reset(QVariantMap());

// Check if service menu is ready
bool isReady = serviceMenu->isReady();

// Get service menu error status
QString error = serviceMenu->getError();
```

### Device Testing API

```cpp
// Access device testing functionality
auto backend = serviceMenu->getContext()["backend"].value<ServiceMenuBackend*>();
if (backend) {
    // Test specific devices
    backend->testBillAcceptor();
    backend->testCoinAcceptor();
    backend->testDispenser();
    backend->testPrinter();
    backend->testHIDDevice();

    // Get device status
    QVariantMap deviceStatus = backend->getDeviceStatus();

    // Get specific device information
    QVariantMap billAcceptorStatus = backend->getBillAcceptorStatus();
    QVariantMap printerStatus = backend->getPrinterStatus();
}
```

### Network Configuration API

```cpp
// Configure network settings
QVariantMap networkConfig;
networkConfig["connectionType"] = "dialup"; // or "ethernet", "wifi"
networkConfig["phoneNumber"] = "+1234567890";
networkConfig["username"] = "admin";
networkConfig["password"] = "secure";
networkConfig["timeout"] = 30;

serviceMenu->notify("configureNetwork", networkConfig);

// Test network connection
QVariantMap testResult = backend->testNetworkConnection();

// Get network status
QVariantMap networkStatus = backend->getNetworkStatus();
```

### Payment System API

```cpp
// Configure payment system
QVariantMap paymentConfig;
paymentConfig["paymentProcessor"] = "humo";
paymentConfig["terminalId"] = "TERM12345";
paymentConfig["merchantId"] = "MERCH67890";
paymentConfig["enableContactless"] = true;

serviceMenu->notify("configurePayment", paymentConfig);

// Test payment system
QVariantMap paymentTestResult = backend->testPaymentSystem();

// Get payment status
QVariantMap paymentStatus = backend->getPaymentStatus();
```

### Key Management API

```cpp
// Manage security keys
QVariantMap keyOperation;
keyOperation["operation"] = "generate"; // or "import", "export", "delete"
keyOperation["keyType"] = "rsa";
keyOperation["keySize"] = 2048;
keyOperation["keyName"] = "service_key";

serviceMenu->notify("manageKeys", keyOperation);

// Get key status
QVariantMap keyStatus = backend->getKeyStatus();

// List available keys
QStringList availableKeys = backend->listAvailableKeys();
```

---

## Integration

### CMake Configuration

The ServiceMenu plugin uses the standard EKiosk plugin CMake configuration:

```cmake
# Plugin definition
ek_add_plugin(service_menu
    FOLDER "plugins"
    SOURCES
        ${SERVICEMENU_SOURCES}    # All C++ source files
        ${SERVICEMENU_UI_FILES}  # Qt UI files
        ${SERVICEMENU_RESOURCES}  # Resource files (QRC)
    QT_MODULES
        Core      # Qt Core module
        Gui       # Qt GUI module
        Widgets   # Qt Widgets module
        Network   # Qt Network module
    DEPENDS
        BasicApplication    # Core application framework
        Connection          # Network connection management
        PluginsSDK          # Plugin system infrastructure
        DriversSDK          # Hardware driver interfaces
        PPSDK               # Payment processor interfaces
        NetworkTaskManager   # Network task management
        KeysUtils           # Cryptographic key utilities
        Log                 # Logging system
        SysUtils            # System utilities
        ek_boost            # Boost libraries wrapper
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src  # Plugin source directory
    INSTALL_DIR
        plugins  # Installation directory
)
```

### Build Process

```bash
# Configure the project (Windows MSVC, Qt5, x64)
cmake --preset win-msvc-qt5-x64

# Build the ServiceMenu plugin
cmake --build build/win-msvc-qt5-x64 --target service_menu

# Build and run tests
cmake --build build/win-msvc-qt5-x64 --target service_menu_test
```

### Plugin Loading

The ServiceMenu plugin is automatically loaded by the EKiosk plugin system:

```cpp
// Plugin is loaded automatically during application startup
// Access through the plugin system:
SDK::Plugin::IPlugin *serviceMenu = environment->getPlugin("service_menu");
```

### Dependency Management

The plugin requires several internal dependencies to be built first:

1. **Core Modules**: BasicApplication, Connection
2. **SDK Modules**: PluginsSDK, DriversSDK, PPSDK
3. **Utility Modules**: NetworkTaskManager, KeysUtils, Log, SysUtils
4. **External**: ek_boost (Boost libraries)

All dependencies are automatically handled by the CMake build system.

---

## Testing

### Test Framework

The ServiceMenu plugin includes comprehensive tests using the EKiosk testing framework with mock kernel infrastructure.

### Test Coverage Areas

- **Plugin Loading**: Factory registration and plugin instantiation
- **Initialization**: Plugin startup and dependency injection
- **Main Window**: Service menu UI functionality
- **Device Testing**: Hardware device testing modules
- **Network Configuration**: Network setup and testing
- **Payment System**: Payment configuration and testing
- **Key Management**: Security key operations
- **Setup Wizard**: First-time setup flow
- **Auto-encashment**: Cash handling functionality
- **Error Handling**: Graceful failure and recovery

### Test Structure

```
tests/plugins/
├── NativeWidgets/
│   └── ServiceMenu/
│       ├── service_menu_test.cpp      # Main test file
│       ├── mock_backend.h/.cpp        # Mock backend implementations
│       ├── test_data/                 # Test data and configurations
│       └── CMakeLists.txt             # Test build configuration
```

### Running Tests

```bash
# Run ServiceMenu plugin tests
cmake --build build/win-msvc-qt5-x64 --target service_menu_test

# Run tests with verbose output
ctest --output-on-failure -R service_menu

# Run all plugin tests
cmake --build build/win-msvc-qt5-x64 --target run_tests
```

### Test Examples

```cpp
#include "../common/PluginTestBase.h"
#include <SDK/Plugins/IPlugin.h>

class ServiceMenuTest : public QObject {
    Q_OBJECT

private slots:
    void testPluginLoading();
    void testPluginInitialization();
    void testMainWindowFunctionality();
    void testDeviceTesting();
    void testNetworkConfiguration();
    void testPaymentSystem();
    void testKeyManagement();
    void testSetupWizard();
    void testAutoEncashment();
    void testErrorHandling();
};

void ServiceMenuTest::testPluginLoading() {
    // Test plugin factory and loading
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory("service_menu");
    QVERIFY(factory != nullptr);
    QCOMPARE(factory->getName(), QString("Service menu native widget"));
    QCOMPARE(factory->getModuleName(), QString("service_menu"));
}

void ServiceMenuTest::testDeviceTesting() {
    // Test device testing functionality with mock hardware
    auto serviceMenu = m_testBase.createPlugin("service_menu");
    QVERIFY(serviceMenu->isReady());

    // Test device operations
    QVariantMap testResult = serviceMenu->getContext()["backend"].value<ServiceMenuBackend*>()
        ->testBillAcceptor();
    QVERIFY(testResult.contains("success"));
}
```

### Test Coverage Requirements

- **100% Coverage**: All public methods must be tested
- **Error Paths**: Test failure scenarios and error handling
- **Integration**: Test plugin integration with other systems
- **Performance**: Test with realistic data volumes
- **Platform**: Test on all supported platforms

---

## Dependencies

### Internal Dependencies

| Dependency             | Purpose                                     | Required |
| ---------------------- | ------------------------------------------- | -------- |
| **BasicApplication**   | Core application framework and utilities    | ✅ Yes   |
| **Connection**         | Network connection management and utilities | ✅ Yes   |
| **PluginsSDK**         | Plugin system infrastructure and interfaces | ✅ Yes   |
| **DriversSDK**         | Hardware driver interfaces and base classes | ✅ Yes   |
| **PPSDK**              | Payment processor interfaces and utilities  | ✅ Yes   |
| **NetworkTaskManager** | Network task management and utilities       | ✅ Yes   |
| **KeysUtils**          | Cryptographic key utilities and management  | ✅ Yes   |
| **Log**                | Logging system and utilities                | ✅ Yes   |
| **SysUtils**           | System utilities and helpers                | ✅ Yes   |
| **ek_boost**           | Boost libraries wrapper and utilities       | ✅ Yes   |

### External Dependencies

| Dependency     | Purpose                    | Version Requirements |
| -------------- | -------------------------- | -------------------- |
| **Qt Core**    | Core Qt functionality      | Qt5 or Qt6           |
| **Qt Gui**     | GUI functionality          | Qt5 or Qt6           |
| **Qt Widgets** | Widget-based UI components | Qt5 or Qt6           |
| **Qt Network** | Network functionality      | Qt5 or Qt6           |

### Platform-Specific Dependencies

| Platform    | Dependency                | Purpose                                 |
| ----------- | ------------------------- | --------------------------------------- |
| **Windows** | Advapi32                  | Advanced Windows API functions          |
| **Windows** | Rasapi32                  | Remote Access Service API (for dial-up) |
| **Linux**   | libudev                   | Device management (optional)            |
| **All**     | Platform-specific drivers | Hardware device support                 |

---

## Troubleshooting

### Common Issues and Solutions

#### Plugin Loading Issues

**Symptom**: Plugin fails to load during application startup
**Possible Causes**:

- Missing dependencies
- Incorrect plugin path
- Version mismatch
- Corrupted plugin files

**Solutions**:

```bash
# Verify plugin file exists
ls plugins/service_menud.dll  # Windows
ls plugins/libservice_menu.so  # Linux

# Check dependencies
ldd plugins/libservice_menu.so  # Linux
dumpbin /DEPENDENTS plugins/service_menud.dll  # Windows

# Verify plugin configuration
check_plugin_config service_menu
```

#### Device Testing Failures

**Symptom**: Device tests fail or devices not detected
**Possible Causes**:

- Hardware not connected
- Drivers not installed
- Permission issues
- Device configuration errors

**Solutions**:

```cpp
// Check device status programmatically
auto backend = serviceMenu->getContext()["backend"].value<ServiceMenuBackend*>();
QVariantMap deviceStatus = backend->getDeviceStatus();

// Verify hardware connections
check_hardware_connections();

// Install required drivers
install_device_drivers();
```

#### Network Configuration Issues

**Symptom**: Network tests fail or connections not established
**Possible Causes**:

- Invalid network settings
- Network unavailable
- Firewall blocking
- Authentication failures

**Solutions**:

```cpp
// Test network connectivity
QVariantMap networkTest = backend->testNetworkConnection();

// Verify network settings
QVariantMap networkConfig = serviceMenu->getConfiguration();
verify_network_settings(networkConfig);

// Check firewall settings
check_firewall_settings();
```

#### UI Rendering Issues

**Symptom**: UI elements not displaying correctly
**Possible Causes**:

- Missing resource files
- Incorrect paths
- Qt style issues
- Graphics driver problems

**Solutions**:

```bash
# Verify resource files
check_resources ServiceMenu

# Test with different Qt styles
export QT_STYLE_OVERRIDE=fusion

# Update graphics drivers
update_graphics_drivers
```

### Debugging Techniques

**Enable Debug Mode**:

```cpp
QVariantMap config = serviceMenu->getConfiguration();
config["debugMode"] = true;
serviceMenu->setConfiguration(config);
```

**Check Logs**:

```bash
# Linux/Mac
tail -f /var/log/ekiosk/service_menu.log

# Windows (Event Viewer)
# Look for EKiosk application logs
```

**Command-line Debugging**:

```bash
# Run with debug output
ekiosk --debug --plugin-debug service_menu

# Test plugin in isolation
test_plugin service_menu --verbose
```

---

## Migration notes

### Version History

| Version | Release Date | Notes                  |
| ------- | ------------ | ---------------------- |
| 1.0     | Current      | Initial stable release |
| 0.9     | -            | Beta release           |
| 0.1-0.8 | -            | Development versions   |

### API Compatibility

- **Backward Compatibility**: ✅ Maintained
- **Forward Compatibility**: ✅ Maintained within major version
- **Breaking Changes**: Documented with migration guides

### Migration from Previous Versions

**From 0.x to 1.0**:

- No breaking changes
- Configuration format unchanged
- API fully compatible
- Recommended upgrade for all users

### Future Migration Plans

- **2.0**: Potential API cleanup and modernization
- **3.0**: Major architecture updates (if needed)
- All breaking changes will be documented with migration guides

---

## Further reading

- [Plugin System Architecture](README.md#plugin-architecture)
- [EKiosk Developer Guide](../../docs/getting-started.md)
- [Testing Guide](../../docs/testing.md)
- [CMake Build System](../../docs/build-guide.md)
- [Qt Plugin System Documentation](https://doc.qt.io/qt-6/plugins-howto.html)

### Related Plugins

- **ScreenMaker**: Screen creation and management
- **Ad**: Advertising display components
- **Hardware Drivers**: Device-specific plugins

### Source Code Reference

- **Main Plugin**: `src/plugins/NativeWidgets/ServiceMenu/`
- **Tests**: `tests/plugins/NativeWidgets/ServiceMenu/`
- **Documentation**: `docs/plugins/servicemenu.md`

---

## Configuration Reference

### Complete Configuration Options

```ini
[ServiceMenu]
; Display options
showAdvancedOptions=false
showDeviceStatus=true
defaultServiceTab=hardware

; Feature toggles
enableHardwareTests=true
enableNetworkTesting=true
autoEncashmentEnabled=false
enablePaymentTesting=true
enableKeyManagement=true

; Debug and logging
debugMode=false
logLevel=normal
logFile=service_menu.log

; UI settings
theme=default
fontSize=medium
animationSpeed=normal

; Network settings
networkTimeout=30
connectionRetries=3
pingInterval=5
```

### Configuration Management

```cpp
// Load configuration from file
serviceMenu->loadConfiguration();

// Save configuration to file
serviceMenu->saveConfiguration();

// Reset to default configuration
serviceMenu->resetConfiguration();

// Validate configuration
bool isValid = serviceMenu->validateConfiguration();
```

---

## Best Practices

### Plugin Usage

- **Initialization**: Always check `isReady()` before using plugin
- **Error Handling**: Handle plugin errors gracefully
- **Configuration**: Save configuration after changes
- **Cleanup**: Properly hide plugin when not in use

### Development

- **Testing**: Maintain 100% test coverage
- **Documentation**: Keep README and docs updated
- **Dependencies**: Document all dependencies
- **Compatibility**: Test on all supported platforms

### Deployment

- **Configuration**: Provide default configuration
- **Dependencies**: Include all required dependencies
- **Testing**: Test on target platforms
- **Documentation**: Include user documentation

---

## Support

For issues and support:

- **GitHub Issues**: Report bugs and feature requests
- **Documentation**: Check for updates and examples
- **Community**: Join the developer community
- **Professional Support**: Contact EKiosk support team

---

## License

The ServiceMenu plugin is licensed under the same terms as the main EKiosk project. See the main project license for details.
