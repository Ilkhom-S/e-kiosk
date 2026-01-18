# Ad Plugin

Advertisement management and payment processing plugin for EKiosk - provides comprehensive advertising content management and sponsored payment functionality.

## Purpose

The Ad Plugin serves as the core advertisement management system for EKiosk, providing:

- **Advertisement Payment Processing**: Handles advertisement-sponsored payment transactions
- **Content Management**: Manages advertisement campaigns and content
- **Campaign Integration**: Links payments to advertisement display and campaigns
- **Remote Content Updates**: Supports remote advertisement content management
- **Statistics Tracking**: Monitors payment and advertisement performance

This plugin enables monetization through sponsored content and provides a complete advertising ecosystem for kiosk devices.

---

## Quick start 🔧

```cpp
// Access Ad plugin through the plugin system
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/IEnvironment.h>

// Get Ad plugin instance
SDK::Plugin::IPlugin *adPlugin = environment->getPlugin("ad_plugin");
if (adPlugin && adPlugin->isReady()) {
    // Access Ad service
    auto adService = environment->getInterface<AdService>("AdService");
    if (adService) {
        // Schedule advertisement campaign
        adService->scheduleCampaign(campaignData);
    }
}
```

---

## Features

### Core Advertisement Modules

- **Payment Factory**: Processes advertisement-sponsored payment transactions
  - Ad-based payment processing
  - Sponsored content integration
  - Payment-to-advertisement linking
  - Transaction management

- **Content Management**: Advertisement campaign and content management
  - Campaign scheduling and management
  - Content storage and retrieval
  - Campaign lifecycle management
  - Content versioning

- **Remote Services**: Remote advertisement content updates
  - Remote content synchronization
  - Content update management
  - Network-based content delivery
  - Update scheduling

- **Statistics Tracking**: Performance monitoring and reporting
  - Campaign performance tracking
  - Payment statistics
  - Content display metrics
  - Reporting and analytics

### Service Components

- **AdService**: Core advertisement service interface
- **PaymentFactory**: Payment processing factory
- **AdRemotePlugin**: Remote content management
- **AdPayment**: Advertisement payment processing
- **AdClient**: Advertisement content client

### Integration Points

- **Payment System**: Integration with EKiosk payment processor
- **Database Service**: Campaign and content storage
- **Network Service**: Remote content updates
- **Event Service**: Campaign notifications and events

---

## Platform support

### Qt Version Compatibility

**Qt Version Support:**

- ✅ Qt5 compatible
- ✅ Qt6 compatible
- No version-specific requirements

### Platform Support Table

- **Windows**: ✅ Full - Complete advertisement functionality
  - Full payment processing support
  - Remote content updates
  - Complete statistics tracking
  - Native Windows integration

- **Linux**: ✅ Full - Complete functionality with platform-specific features
  - Full payment processing
  - Remote content management
  - Statistics and reporting
  - Platform-specific optimizations

- **macOS**: 🔬 TODO - Limited testing, core functionality should work
  - Basic functionality tested
  - Requires additional testing for production use
  - May need platform-specific adjustments

---

## Configuration

The Ad plugin supports comprehensive runtime configuration:

```cpp
// Get current configuration
QVariantMap config = adPlugin->getConfiguration();

// Update configuration options
config["displayInterval"] = 300; // 5 minutes
config["maxCampaigns"] = 10;
config["contentPath"] = "/opt/ekiosk/ads";
config["enableRemoteUpdates"] = true;
config["statisticsEnabled"] = true;

// Apply configuration
adPlugin->setConfiguration(config);

// Save configuration permanently
adPlugin->saveConfiguration();
```

### Configuration Options Reference

| Option                | Type   | Default           | Description                                       | Example Values                       |
| --------------------- | ------ | ----------------- | ------------------------------------------------- | ------------------------------------ |
| `displayInterval`     | int    | 300               | Interval between advertisement displays (seconds) | 60, 300, 600                         |
| `maxCampaigns`        | int    | 10                | Maximum number of active campaigns                | 5, 10, 20                            |
| `contentPath`         | string | "/opt/ekiosk/ads" | Path to advertisement content storage             | "/var/ekiosk/ads", "C:\\ekiosk\\ads" |
| `enableRemoteUpdates` | bool   | true              | Enable remote content updates                     | true, false                          |
| `statisticsEnabled`   | bool   | true              | Enable statistics tracking and reporting          | true, false                          |
| `defaultCampaign`     | string | ""                | Default campaign to display                       | "summer_sale", "holiday_special"     |
| `autoRotate`          | bool   | false             | Automatically rotate campaigns                    | true, false                          |

### Configuration File

Configuration is stored in the plugin's configuration file:

```
[AdPlugin]
displayInterval=300
maxCampaigns=10
contentPath=/opt/ekiosk/ads
enableRemoteUpdates=true
statisticsEnabled=true
defaultCampaign=
autoRotate=false
```

---

## Usage / API highlights

### Main Ad Plugin Operations

```cpp
// Show advertisement plugin
adPlugin->show();

// Hide advertisement plugin
adPlugin->hide();

// Reset advertisement plugin state
adPlugin->reset(QVariantMap());

// Check if advertisement plugin is ready
bool isReady = adPlugin->isReady();

// Get advertisement plugin error status
QString error = adPlugin->getError();
```

### Campaign Management API

```cpp
// Access Ad service
auto adService = environment->getInterface<AdService>("AdService");
if (adService) {
    // Schedule advertisement campaign
    QVariantMap campaignData;
    campaignData["campaignId"] = "summer_sale_2024";
    campaignData["startDate"] = QDateTime::currentDateTime();
    campaignData["endDate"] = QDateTime::currentDateTime().addDays(30);
    campaignData["contentId"] = "banner_summer";
    campaignData["priority"] = 1;

    bool success = adService->scheduleCampaign(campaignData);

    // Cancel advertisement campaign
    bool cancelled = adService->cancelCampaign("summer_sale_2024");

    // Get active campaigns
    QList<QVariantMap> activeCampaigns = adService->getActiveCampaigns();
}
```

### Content Management API

```cpp
// Load advertisement content
QVariant content = adService->loadAdvertisement("banner_summer");

// Get current advertisement
QVariantMap currentAd = adService->getCurrentAdvertisement();

// Display sponsored advertisement
bool displayed = adService->displaySponsoredAd("banner_summer");
```

### Statistics API

```cpp
// Get campaign statistics
QVariantMap stats = adService->getStatistics("summer_sale_2024");

// Reset campaign statistics
bool resetSuccess = adService->resetStatistics("summer_sale_2024");

// Get overall statistics
QVariantMap overallStats = adService->getOverallStatistics();
```

### Payment Integration API

```cpp
// Process advertisement-sponsored payment
QVariantMap paymentData;
paymentData["paymentType"] = "ad";
paymentData["amount"] = 1000; // in smallest currency units
paymentData["currency"] = "UZS";
paymentData["adId"] = "banner_summer";
paymentData["campaignId"] = "summer_sale_2024";

auto payment = paymentFactory->createPayment("ad");
if (payment) {
    payment->setParameters(paymentData);
    bool paymentSuccess = payment->process();
    paymentFactory->releasePayment(payment);
}
```

---

## Integration

### CMake Configuration

The Ad plugin uses the standard EKiosk plugin CMake configuration:

```cmake
# From src/plugins/Ad/CMakeLists.txt
include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

set(AD_PLUGIN_SOURCES
    src/AdPluginFactory.cpp
    src/AdPluginFactory.h
    src/AdPluginImpl.cpp
    src/AdPluginImpl.h
    src/PaymentFactory.cpp
    src/PaymentFactory.h
    src/PaymentFactoryBase.cpp
    src/PaymentFactoryBase.h
    src/AdService.cpp
    src/AdService.h
    src/AdPayment.cpp
    src/AdPayment.h
    src/AdRemotePlugin.cpp
    src/AdRemotePlugin.h
    src/AdSourcePlugin.cpp
    src/AdSourcePlugin.h
)

ek_add_plugin(ad_plugin
    FOLDER "plugins"
    SOURCES ${AD_PLUGIN_SOURCES}
    QT_MODULES Core Network
    DEPENDS
        BasicApplication
        Connection
        PluginsSDK
        PaymentBase
        NetworkTaskManager
        SysUtils
        ek_boost
    INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src
    INSTALL_DIR plugins
)
```

### Build Dependencies

- **Core Modules**: BasicApplication, Connection, PluginsSDK
- **Payment Modules**: PaymentBase
- **Utility Modules**: NetworkTaskManager, SysUtils
- **External**: ek_boost

### Building the Plugin

```bash
# Configure and build the entire project (includes Ad plugin)
cmake --preset win-msvc-qt5-x64
cmake --build build/win-msvc-qt5-x64 --target ad_plugin

# Build just the Ad plugin
cmake --build build/win-msvc-qt5-x64 --target ad_plugin
```

### Plugin Loading

The Ad plugin is automatically loaded by the EKiosk plugin system:

```cpp
// Plugin is loaded automatically during application startup
// Access through the plugin system:
SDK::Plugin::IPlugin *adPlugin = environment->getPlugin("ad_plugin");

// Access through payment factory:
auto paymentFactory = environment->getInterface<PaymentFactory>("AdPayments");
```

### Dependency Management

The plugin requires several internal dependencies to be built first:

1. **Core Modules**: BasicApplication, Connection
2. **SDK Modules**: PluginsSDK, PaymentBase
3. **Utility Modules**: NetworkTaskManager, SysUtils
4. **External**: ek_boost (Boost libraries)

All dependencies are automatically handled by the CMake build system.

---

## Testing

### Test Framework

The Ad plugin includes comprehensive tests using the EKiosk testing framework with mock kernel infrastructure.

### Test Coverage Areas

- **Plugin Loading**: Factory registration and plugin instantiation
- **Initialization**: Plugin startup and dependency injection
- **Payment Processing**: Advertisement payment transactions
- **Campaign Management**: Campaign scheduling and management
- **Content Management**: Content loading and display
- **Statistics Tracking**: Performance monitoring
- **Remote Updates**: Content synchronization
- **Error Handling**: Graceful failure and recovery

### Test Structure

```text
tests/plugins/
├── Ad/
│   ├── ad_plugin_test.cpp      # Main test file
│   ├── mock_ad_service.h/.cpp  # Mock service implementations
│   ├── test_data/              # Test campaigns and content
│   └── CMakeLists.txt          # Test build configuration
```

### Running Tests

```bash
# Run Ad plugin tests
cmake --build build/win-msvc-qt5-x64 --target ad_plugin_test

# Run tests with verbose output
ctest --output-on-failure -R ad_plugin

# Run all plugin tests
cmake --build build/win-msvc-qt5-x64 --target run_tests
```

### Test Examples

```cpp
#include "../common/PluginTestBase.h"
#include <SDK/Plugins/IPlugin.h>
#include <SDK/PaymentProcessor/Payment/IPaymentFactory.h>

class AdPluginTest : public QObject {
    Q_OBJECT

private slots:
    void testPluginLoading();
    void testPluginInitialization();
    void testPaymentProcessing();
    void testCampaignManagement();
    void testContentManagement();
    void testStatisticsTracking();
    void testRemoteUpdates();
    void testErrorHandling();
};

void AdPluginTest::testPluginLoading() {
    // Test plugin factory and loading
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory("ad_plugin");
    QVERIFY(factory != nullptr);
    QCOMPARE(factory->getName(), QString("Ad Plugin"));
    QCOMPARE(factory->getModuleName(), QString("ad_plugin"));
}

void AdPluginTest::testPaymentProcessing() {
    // Test advertisement payment processing
    auto paymentFactory = m_testBase.getInterface<PaymentFactory>("AdPayments");
    QVERIFY(paymentFactory != nullptr);

    // Test payment creation and processing
    auto payment = paymentFactory->createPayment("ad");
    QVERIFY(payment != nullptr);

    QVariantMap paymentData;
    paymentData["amount"] = 1000;
    paymentData["adId"] = "test_banner";

    payment->setParameters(paymentData);
    bool success = payment->process();
    QVERIFY(success);

    paymentFactory->releasePayment(payment);
}
```

### Test Coverage Requirements

- **100% Coverage**: All public methods must be tested
- **Error Paths**: Test failure scenarios and error handling
- **Integration**: Test plugin integration with payment and ad services
- **Performance**: Test with realistic campaign data volumes
- **Platform**: Test on all supported platforms

---

## Dependencies

### Internal Dependencies

| Dependency             | Purpose                                        | Required |
| ---------------------- | ---------------------------------------------- | -------- |
| **BasicApplication**   | Core application framework and utilities       | ✅ Yes   |
| **Connection**         | Network connection management                  | ✅ Yes   |
| **PluginsSDK**         | Plugin system infrastructure and interfaces    | ✅ Yes   |
| **PaymentBase**        | Payment processing base classes and interfaces | ✅ Yes   |
| **NetworkTaskManager** | Network task management and utilities          | ✅ Yes   |
| **SysUtils**           | System utilities and helpers                   | ✅ Yes   |
| **ek_boost**           | Boost libraries wrapper and utilities          | ✅ Yes   |

### External Dependencies

| Dependency     | Purpose                                  | Version Requirements |
| -------------- | ---------------------------------------- | -------------------- |
| **Qt Core**    | Core Qt functionality                    | Qt5 or Qt6           |
| **Qt Network** | Network functionality for remote updates | Qt5 or Qt6           |

### Platform-Specific Dependencies

| Platform | Dependency       | Purpose                       |
| -------- | ---------------- | ----------------------------- |
| **All**  | Database Service | Campaign and content storage  |
| **All**  | Network Service  | Remote content updates        |
| **All**  | Payment Service  | Sponsored content integration |
| **All**  | Event Service    | Campaign notifications        |

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
ls plugins/ad_plugind.dll  # Windows
ls plugins/libad_plugin.so  # Linux

# Check dependencies
ldd plugins/libad_plugin.so  # Linux
dumpbin /DEPENDENTS plugins/ad_plugind.dll  # Windows

# Verify plugin configuration
check_plugin_config ad_plugin
```

#### Campaign Display Issues

**Symptom**: Advertisement campaigns not displaying
**Possible Causes**:

- Campaign not scheduled
- Content not available
- Display interval too long
- Campaign expired

**Solutions**:

```cpp
// Check active campaigns
auto adService = environment->getInterface<AdService>("AdService");
QList<QVariantMap> activeCampaigns = adService->getActiveCampaigns();

// Verify campaign scheduling
QVariantMap campaign = activeCampaigns.first();
checkCampaignSchedule(campaign);

// Check content availability
QVariant content = adService->getContent(campaign["contentId"].toString());
```

#### Payment Integration Issues

**Symptom**: Advertisement payments not processing
**Possible Causes**:

- Invalid payment configuration
- Payment service unavailable
- Advertisement ID not found
- Campaign not active

**Solutions**:

```cpp
// Verify payment factory
auto paymentFactory = environment->getInterface<PaymentFactory>("AdPayments");
if (!paymentFactory) {
    // Payment factory not available
    initializePaymentFactory();
}

// Check payment parameters
QVariantMap paymentData;
paymentData["adId"] = "valid_ad_id";
paymentData["campaignId"] = "active_campaign";

// Test payment processing
auto payment = paymentFactory->createPayment("ad");
payment->setParameters(paymentData);
bool success = payment->process();
```

#### Remote Update Issues

**Symptom**: Remote content updates not working
**Possible Causes**:

- Network connectivity issues
- Remote service unavailable
- Permission issues
- Content format errors

**Solutions**:

```bash
# Check network connectivity
ping remote.ad.server

# Verify remote service
check_ad_remote_service

# Test content synchronization
sync_ad_content --verbose
```

### Debugging Techniques

**Enable Debug Mode**:

```cpp
QVariantMap config = adPlugin->getConfiguration();
config["debugMode"] = true;
adPlugin->setConfiguration(config);
```

**Check Logs**:

```bash
# Linux/Mac
tail -f /var/log/ekiosk/ad_plugin.log

# Windows (Event Viewer)
# Look for EKiosk Ad Plugin logs
```

**Command-line Debugging**:

```bash
# Run with debug output
ekiosk --debug --plugin-debug ad_plugin

# Test plugin in isolation
test_plugin ad_plugin --verbose
```

---

## Migration notes

### Version History

| Version | Release Date | Notes                                 |
| ------- | ------------ | ------------------------------------- |
| 1.0     | Current      | Initial stable release                |
| 0.9     | -            | Beta release with payment integration |
| 0.1-0.8 | -            | Development versions                  |

### API Compatibility

- **Backward Compatibility**: ✅ Maintained
- **Forward Compatibility**: ✅ Maintained within major version
- **Breaking Changes**: Documented with migration guides

### Migration from Previous Versions

**From Legacy TerminalClient**:

- Updated to standard EKiosk plugin architecture
- Implemented IPluginFactory/IPlugin pattern
- Added proper Qt5/Qt6 compatibility
- Integrated with EKiosk services
- Updated coding standards with Russian comments
- Centralized metadata in AdPluginFactory.cpp

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
- [Ad Service Documentation](../../docs/services/ad.md)
- [Payment Service Documentation](../../docs/services/payment.md)
- [EKiosk Developer Guide](../../docs/getting-started.md)
- [Testing Guide](../../docs/testing.md)

### Related Plugins

- **ServiceMenu**: Service and maintenance interface
- **Payment Plugins**: Various payment processing plugins
- **ScreenMaker**: Screen creation and management

### Source Code Reference

- **Main Plugin**: `src/plugins/Ad/`
- **Tests**: `tests/plugins/Ad/`
- **Documentation**: `docs/plugins/ad.md`

---

## Configuration Reference

### Complete Configuration Options

```ini
[AdPlugin]
; Display options
displayInterval=300
maxCampaigns=10
autoRotate=false

; Content management
contentPath=/opt/ekiosk/ads
defaultCampaign=

; Remote updates
enableRemoteUpdates=true
updateInterval=3600
remoteServer=https://ad-server.example.com

; Statistics and debugging
statisticsEnabled=true
debugMode=false
logLevel=normal
logFile=ad_plugin.log
```

### Configuration Management

```cpp
// Load configuration from file
adPlugin->loadConfiguration();

// Save configuration to file
adPlugin->saveConfiguration();

// Reset to default configuration
adPlugin->resetConfiguration();

// Validate configuration
bool isValid = adPlugin->validateConfiguration();
```

---

## Best Practices

### Plugin Usage

- **Initialization**: Always check `isReady()` before using plugin
- **Error Handling**: Handle plugin errors gracefully
- **Configuration**: Save configuration after changes
- **Cleanup**: Properly release payments and resources

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

The Ad plugin is licensed under the same terms as the main EKiosk project. See the main project license for details.
