# Ad Plugin Documentation

Comprehensive documentation for the Ad (Advertisement) plugin.

## Title & Short Summary

**Ad Plugin** - Advertisement management and payment processing plugin for EKiosk devices.

## Purpose

The Ad Plugin provides a complete advertisement management system for EKiosk, enabling monetization through sponsored content and providing comprehensive advertising functionality. It serves as the core component for:

- **Advertisement Display**: Managing and displaying advertisement campaigns
- **Sponsored Payments**: Processing payments for advertisement-sponsored transactions
- **Content Management**: Handling advertisement content lifecycle
- **Campaign Tracking**: Monitoring and reporting on advertisement performance
- **Remote Updates**: Synchronizing content with remote advertisement servers

This plugin is essential for implementing advertisement-based revenue models in kiosk deployments.

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
        QVariantMap campaignData;
        campaignData["campaignId"] = "summer_sale";
        campaignData["contentId"] = "banner_summer";
        campaignData["priority"] = 1;

        bool success = adService->scheduleCampaign(campaignData);

        // Process advertisement payment
        auto paymentFactory = environment->getInterface<PaymentFactory>("AdPayments");
        if (paymentFactory) {
            auto payment = paymentFactory->createPayment("ad");
            // Configure and process payment...
        }
    }
}
```text

---

## Features

### Core Advertisement Functionality

- **Payment Processing**: Advertisement-sponsored payment transactions
  - Ad-based payment factory implementation
  - Sponsored content payment processing
  - Payment-to-advertisement linking
  - Transaction management and reporting

- **Campaign Management**: Advertisement campaign lifecycle
  - Campaign scheduling and activation
  - Content association and management
  - Priority-based campaign selection
  - Campaign expiration and cleanup

- **Content Management**: Advertisement content handling
  - Content storage and retrieval
  - Content versioning and updates
  - Content validation and verification
  - Content lifecycle management

- **Remote Services**: Network-based content management
  - Remote content synchronization
  - Content update scheduling
  - Network-based content delivery
  - Update conflict resolution

- **Statistics & Reporting**: Performance monitoring
  - Campaign performance tracking
  - Payment statistics and analytics
  - Content display metrics
  - Comprehensive reporting

### Plugin Architecture

- **AdService**: Core advertisement service interface
- **PaymentFactory**: Payment processing factory implementation
- **AdRemotePlugin**: Remote content management
- **AdPayment**: Advertisement payment processing
- **AdClient**: Advertisement content client
- **AdSourcePlugin**: Content source management

### Integration Points

- **Payment System**: Integration with EKiosk payment processor
- **Database Service**: Campaign and content persistence
- **Network Service**: Remote content synchronization
- **Event Service**: Campaign notifications and events
- **Logging Service**: Comprehensive logging and diagnostics

---

## Platform support

### Qt Version Compatibility

**Qt Version Support:**

- ✅ Qt5 compatible
- ✅ Qt6 compatible
- No version-specific requirements

The Ad Plugin is designed to work seamlessly with both Qt5 and Qt6 without requiring version-specific code or conditional compilation.

### Platform Support Table

- **Windows**: ✅ Full - Complete advertisement functionality
  - Full payment processing support
  - Remote content updates
  - Complete statistics tracking
  - Native Windows integration
  - Active Directory integration (optional)

- **Linux**: ✅ Full - Complete functionality with platform-specific optimizations
  - Full payment processing
  - Remote content management
  - Statistics and reporting
  - Platform-specific optimizations
  - Systemd service integration

- **macOS**: 🔬 TODO - Limited testing, core functionality should work
  - Basic functionality tested
  - Requires additional testing for production use
  - May need platform-specific adjustments
  - Sandboxing considerations

---

## Accessing Services

The Ad Plugin provides access to various system services through the plugin environment and service interfaces.

### Environment & Logging

```cpp
// Constructor receives IEnvironment
AdPluginImpl::AdPluginImpl(SDK::Plugin::IEnvironment *aEnvironment, const QString &aInstancePath)
    : mEnvironment(aEnvironment), mInstancePath(aInstancePath) {
}

// Get logger for this plugin
ILog *log = mEnvironment->getLog("AdPlugin");
LOG(log, LogLevel::Normal, "AdPlugin initialized");
```text

### Core Services

```cpp
// Access core services through environment
SDK::PaymentProcessor::ICore *core = dynamic_cast<SDK::PaymentProcessor::ICore*>(
    mEnvironment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

// Access database service
auto databaseService = core->getDatabaseService();

// Access network service
auto networkService = core->getNetworkService();
```text

### Plugin-Specific Services

```cpp
// Access AdService
auto adService = mEnvironment->getInterface<AdService>("AdService");
if (adService) {
    // Campaign management
    bool success = adService->scheduleCampaign(campaignData);

    // Content management
    QVariant content = adService->loadAdvertisement("banner_id");

    // Statistics
    QVariantMap stats = adService->getStatistics("campaign_id");
}

// Access PaymentFactory
auto paymentFactory = mEnvironment->getInterface<PaymentFactory>("AdPayments");
if (paymentFactory) {
    // Payment processing
    auto payment = paymentFactory->createPayment("ad");
    payment->setParameters(paymentData);
    bool paymentSuccess = payment->process();
}
```text

### Service Availability

- **Environment**: Always available (passed in constructor)
- **Core Services**: Available after plugin initialization
- **AdService**: Available after service initialization
- **PaymentFactory**: Available after payment system initialization
- **Logging**: Always available through environment

### Error Handling

```cpp
try {
    auto adService = mEnvironment->getInterface<AdService>("AdService");
    if (adService) {
        // Use ad service
        bool success = adService->scheduleCampaign(campaignData);
        if (!success) {
            // Handle campaign scheduling failure
            LOG(mEnvironment->getLog("AdPlugin"), LogLevel::Error,
                "Failed to schedule campaign");
        }
    }
} catch (const SDK::PaymentProcessor::ServiceIsNotImplemented &e) {
    LOG(mEnvironment->getLog("AdPlugin"), LogLevel::Error,
        QString("AdService not available: %1").arg(e.what()));
}
```text

---

## Configuration

### Plugin Configuration

The Ad Plugin supports comprehensive runtime configuration through the standard plugin configuration interface:

```cpp
// Get current configuration
QVariantMap config = adPlugin->getConfiguration();

// Update configuration options
config["displayInterval"] = 300; // seconds
config["maxCampaigns"] = 15;
config["contentPath"] = "/var/ekiosk/ads";
config["enableRemoteUpdates"] = true;
config["statisticsEnabled"] = true;
config["autoRotate"] = true;

// Apply configuration
adPlugin->setConfiguration(config);

// Save configuration permanently
adPlugin->saveConfiguration();
```text

### Configuration Options Reference

| Option                | Type   | Default           | Description                                       | Example Values                       | Since |
| --------------------- | ------ | ----------------- | ------------------------------------------------- | ------------------------------------ | ----- |
| `displayInterval`     | int    | 300               | Interval between advertisement displays (seconds) | 60, 300, 600                         | 1.0   |
| `maxCampaigns`        | int    | 10                | Maximum number of active campaigns                | 5, 10, 20                            | 1.0   |
| `contentPath`         | string | "/opt/ekiosk/ads" | Path to advertisement content storage             | "/var/ekiosk/ads", "C:\\ekiosk\\ads" | 1.0   |
| `enableRemoteUpdates` | bool   | true              | Enable remote content updates                     | true, false                          | 1.0   |
| `statisticsEnabled`   | bool   | true              | Enable statistics tracking and reporting          | true, false                          | 1.0   |
| `defaultCampaign`     | string | ""                | Default campaign to display                       | "summer_sale", "holiday_special"     | 1.0   |
| `autoRotate`          | bool   | false             | Automatically rotate campaigns                    | true, false                          | 1.0   |
| `updateInterval`      | int    | 3600              | Remote update check interval (seconds)            | 1800, 3600, 7200                     | 1.0   |
| `remoteServer`        | string | ""                | Remote advertisement server URL                   | "<https://ad-server.example.com>"    | 1.0   |

### Configuration File Format

Configuration is stored in INI format:

```ini
[AdPlugin]
; Display and rotation settings
displayInterval=300
maxCampaigns=10
autoRotate=false
defaultCampaign=summer_sale

; Content management
contentPath=/opt/ekiosk/ads

; Remote update settings
enableRemoteUpdates=true
updateInterval=3600
remoteServer=https://ad-server.example.com

; Statistics and debugging
statisticsEnabled=true
debugMode=false
logLevel=normal
logFile=/var/log/ekiosk/ad_plugin.log
```text

### Configuration Management API

```cpp
// Load configuration from file
bool loaded = adPlugin->loadConfiguration();

// Save configuration to file
bool saved = adPlugin->saveConfiguration();

// Reset to default configuration
adPlugin->resetConfiguration();

// Validate configuration
bool isValid = adPlugin->validateConfiguration();

// Get specific configuration values
int displayInterval = config["displayInterval"].toInt();
QString contentPath = config["contentPath"].toString();
```text

---

## Usage / API highlights

### Main Plugin Operations

```cpp
// Plugin lifecycle management
bool initialized = adPlugin->initialize();
bool started = adPlugin->start();
bool stopped = adPlugin->stop();

// Plugin state management
adPlugin->show();
adPlugin->hide();
adPlugin->reset(QVariantMap());

// Plugin status
bool isReady = adPlugin->isReady();
QString error = adPlugin->getError();
QVariantMap context = adPlugin->getContext();
```text

### Campaign Management API

```cpp
// Schedule new advertisement campaign
QVariantMap campaignData;
campaignData["campaignId"] = "summer_sale_2024";
campaignData["contentId"] = "banner_summer";
campaignData["startDate"] = QDateTime::currentDateTime();
campaignData["endDate"] = QDateTime::currentDateTime().addDays(30);
campaignData["priority"] = 1;
campaignData["targetAudience"] = "all";
campaignData["maxDisplays"] = 1000;

bool scheduled = adService->scheduleCampaign(campaignData);

// Cancel existing campaign
bool cancelled = adService->cancelCampaign("summer_sale_2024");

// Update campaign
QVariantMap updateData;
updateData["priority"] = 2;
updateData["maxDisplays"] = 2000;
bool updated = adService->updateCampaign("summer_sale_2024", updateData);

// Get campaign information
QVariantMap campaignInfo = adService->getCampaign("summer_sale_2024");
QList<QVariantMap> activeCampaigns = adService->getActiveCampaigns();
QList<QVariantMap> allCampaigns = adService->getAllCampaigns();
```text

### Content Management API

```cpp
// Load advertisement content
QVariant content = adService->loadAdvertisement("banner_summer");
if (content.isValid()) {
    // Content loaded successfully
    QString contentType = content.typeName();
    QByteArray contentData = content.toByteArray();
}

// Get current advertisement
QVariantMap currentAd = adService->getCurrentAdvertisement();
QString currentAdId = currentAd["adId"].toString();
QString currentContentId = currentAd["contentId"].toString();

// Display sponsored advertisement
QVariantMap displayParams;
displayParams["displayDuration"] = 15; // seconds
displayParams["transitionEffect"] = "fade";
bool displayed = adService->displaySponsoredAd("banner_summer", displayParams);

// Manage content
bool contentAdded = adService->addContent("new_banner", contentData);
bool contentRemoved = adService->removeContent("old_banner");
QList<QString> availableContent = adService->listAvailableContent();
```text

### Payment Processing API

```cpp
// Create advertisement payment
QVariantMap paymentData;
paymentData["paymentType"] = "ad";
paymentData["amount"] = 1500; // in smallest currency units
paymentData["currency"] = "UZS";
paymentData["adId"] = "banner_summer";
paymentData["campaignId"] = "summer_sale_2024";
paymentData["merchantId"] = "MERCH12345";
paymentData["terminalId"] = "TERM67890";
paymentData["transactionId"] = generateTransactionId();
paymentData["sponsoredContent"] = true;

auto payment = paymentFactory->createPayment("ad");
if (payment) {
    // Configure payment
    payment->setParameters(paymentData);

    // Process payment
    bool paymentSuccess = payment->process();
    if (paymentSuccess) {
        // Payment processed successfully
        QVariantMap result = payment->getResult();
        QString transactionId = result["transactionId"].toString();

        // Link payment to advertisement display
        adService->recordSponsoredDisplay("banner_summer", transactionId);
    }

    // Release payment object
    paymentFactory->releasePayment(payment);
}

// Payment status and management
QVariantMap paymentStatus = payment->getStatus();
bool canRefund = payment->canRefund();
bool refundSuccess = payment->refund();
```text

### Statistics & Reporting API

```cpp
// Get campaign statistics
QVariantMap campaignStats = adService->getStatistics("summer_sale_2024");
int displays = campaignStats["displays"].toInt();
int clicks = campaignStats["clicks"].toInt();
int conversions = campaignStats["conversions"].toInt();
double ctr = campaignStats["ctr"].toDouble(); // click-through rate
QVariantMap revenue = campaignStats["revenue"].toMap();

// Get content statistics
QVariantMap contentStats = adService->getContentStatistics("banner_summer");
int contentDisplays = contentStats["displays"].toInt();
double engagementRate = contentStats["engagementRate"].toDouble();

// Get overall statistics
QVariantMap overallStats = adService->getOverallStatistics();
int totalCampaigns = overallStats["totalCampaigns"].toInt();
int activeCampaigns = overallStats["activeCampaigns"].toInt();
double totalRevenue = overallStats["totalRevenue"].toDouble();

// Reset statistics
bool statsReset = adService->resetStatistics("summer_sale_2024");
bool allStatsReset = adService->resetAllStatistics();

// Export statistics
QByteArray csvData = adService->exportStatistics("csv");
QByteArray jsonData = adService->exportStatistics("json");
bool exportSuccess = adService->exportStatisticsToFile("report.csv", "csv");
```text

### Remote Content Management API

```cpp
// Check for remote updates
bool updatesAvailable = adService->checkForRemoteUpdates();
int availableUpdates = adService->getAvailableUpdateCount();

// Download and apply updates
bool downloadSuccess = adService->downloadRemoteUpdates();
bool applySuccess = adService->applyRemoteUpdates();

// Manual content synchronization
QVariantMap syncParams;
syncParams["forceSync"] = true;
syncParams["contentTypes"] = QStringList() << "banners" << "videos";
bool syncSuccess = adService->synchronizeRemoteContent(syncParams);

// Remote content information
QVariantMap remoteInfo = adService->getRemoteContentInfo();
QString remoteServer = remoteInfo["server"].toString();
QDateTime lastSync = remoteInfo["lastSync"].toDateTime();
int remoteContentCount = remoteInfo["contentCount"].toInt();

// Content update callbacks
connect(adService, &AdService::contentUpdated,
        this, &MyClass::handleContentUpdate);
connect(adService, &AdService::updateFailed,
        this, &MyClass::handleUpdateFailure);
```text

---

## Integration

### CMake Configuration

The Ad Plugin uses the standard EKiosk plugin CMake configuration with comprehensive dependency management:

```cmake
# Plugin source files
set(AD_PLUGIN_SOURCES
    # Core plugin files
    src/AdPluginFactory.cpp
    src/AdPluginFactory.h
    src/AdPluginImpl.cpp
    src/AdPluginImpl.h

    # Payment processing
    src/PaymentFactory.cpp
    src/PaymentFactory.h
    src/PaymentFactoryBase.cpp
    src/PaymentFactoryBase.h
    src/AdPayment.cpp
    src/AdPayment.h

    # Service implementation
    src/AdService.cpp
    src/AdService.h

    # Remote management
    src/AdRemotePlugin.cpp
    src/AdRemotePlugin.h

    # Content sources
    src/AdSourcePlugin.cpp
    src/AdSourcePlugin.h
)

# Plugin definition
ek_add_plugin(ad_plugin
    FOLDER "plugins"

    # Source files
    SOURCES ${AD_PLUGIN_SOURCES}

    # Required Qt modules
    QT_MODULES
        Core      # Core Qt functionality
        Network   # Network functionality for remote updates

    # Internal dependencies
    DEPENDS
        BasicApplication    # Core application framework
        Connection          # Network connection management
        PluginsSDK          # Plugin system infrastructure
        PaymentBase         # Payment processing base classes
        NetworkTaskManager   # Network task management
        SysUtils            # System utilities
        ek_boost            # Boost libraries wrapper

    # Include directories
    INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src

    # Installation directory
    INSTALL_DIR
        plugins
)
```text

### Build Process

```bash
# Configure the project with specific preset
cmake --preset win-msvc-qt5-x64

# Build the Ad plugin specifically
cmake --build build/win-msvc-qt5-x64 --target ad_plugin

# Build with verbose output for debugging
cmake --build build/win-msvc-qt5-x64 --target ad_plugin --verbose

# Clean and rebuild
cmake --build build/win-msvc-qt5-x64 --target clean
cmake --build build/win-msvc-qt5-x64 --target ad_plugin
```text

### Plugin Loading Sequence

1. **Discovery**: Qt plugin system scans plugin directories
2. **Registration**: Ad plugin registers with EKiosk plugin system
3. **Instantiation**: Plugin factory creates plugin instances
4. **Initialization**: Plugin receives environment and initializes
5. **Service Registration**: AdService registers with core services
6. **Payment Registration**: PaymentFactory registers with payment system
7. **Operation**: Plugin becomes available for use

### Dependency Management

The Ad Plugin has a well-defined dependency hierarchy:

```mermaid
graph TD
    AdPlugin --> BasicApplication
    AdPlugin --> Connection
    AdPlugin --> PluginsSDK
    AdPlugin --> PaymentBase
    AdPlugin --> NetworkTaskManager
    AdPlugin --> SysUtils
    AdPlugin --> ek_boost

    PaymentBase --> PaymentProcessor
    NetworkTaskManager --> NetworkService
    SysUtils --> CoreUtilities
```text

### Integration with EKiosk Services

```cpp
// Service integration during plugin initialization
bool AdPluginImpl::initialize() {
    // Register with core services
    auto core = mEnvironment->getInterface<SDK::PaymentProcessor::ICore>();
    if (core) {
        // Register AdService
        core->registerService("AdService", mAdService);

        // Register PaymentFactory
        core->registerPaymentFactory("AdPayments", mPaymentFactory);

        // Register event handlers
        core->registerEventHandler("ad.display", this, &AdPluginImpl::handleAdDisplay);
        core->registerEventHandler("campaign.start", this, &AdPluginImpl::handleCampaignStart);
    }

    // Initialize remote content management
    mRemotePlugin->initialize();

    // Load configuration
    loadConfiguration();

    return true;
}
```text

---

## Testing

### Test Framework Architecture

The Ad Plugin includes a comprehensive testing framework using the EKiosk mock kernel infrastructure:

```text
tests/plugins/Ad/
├── ad_plugin_test.cpp          # Main test suite
├── mock_ad_service.h/.cpp      # Mock AdService implementation
├── mock_payment_factory.h/.cpp # Mock PaymentFactory implementation
├── test_campaigns/             # Test campaign data
├── test_content/               # Test advertisement content
├── test_configs/               # Test configuration files
└── CMakeLists.txt              # Test build configuration
```text

### Test Coverage Matrix

| Component           | Unit Tests | Integration Tests | Error Paths | Performance Tests |
| ------------------- | ---------- | ----------------- | ----------- | ----------------- |
| Plugin Loading      | ✅         | ✅                | ✅          | ❌                |
| Configuration       | ✅         | ✅                | ✅          | ❌                |
| Campaign Management | ✅         | ✅                | ✅          | ✅                |
| Content Management  | ✅         | ✅                | ✅          | ✅                |
| Payment Processing  | ✅         | ✅                | ✅          | ✅                |
| Statistics Tracking | ✅         | ✅                | ✅          | ❌                |
| Remote Updates      | ✅         | ✅                | ✅          | ✅                |
| Error Handling      | ✅         | ✅                | ✅          | ❌                |

### Test Implementation Examples

```cpp
#include "../common/PluginTestBase.h"
#include <SDK/Plugins/IPlugin.h>
#include <SDK/PaymentProcessor/Payment/IPaymentFactory.h>
#include <SDK/PaymentProcessor/Core/IService.h>

class AdPluginTest : public QObject {
    Q_OBJECT

private slots:
    // Core functionality tests
    void testPluginLoading();
    void testPluginInitialization();
    void testConfigurationManagement();

    // Campaign management tests
    void testCampaignScheduling();
    void testCampaignCancellation();
    void testCampaignUpdates();
    void testCampaignExpiration();

    // Content management tests
    void testContentLoading();
    void testContentDisplay();
    void testContentRotation();
    void testContentValidation();

    // Payment processing tests
    void testPaymentCreation();
    void testPaymentProcessing();
    void testPaymentRefunds();
    void testSponsoredContent();

    // Statistics tests
    void testStatisticsTracking();
    void testStatisticsExport();
    void testStatisticsReset();

    // Remote management tests
    void testRemoteUpdates();
    void testContentSynchronization();
    void testUpdateConflictResolution();

    // Error handling tests
    void testErrorConditions();
    void testGracefulFailure();
    void testRecoveryScenarios();
};

void AdPluginTest::testCampaignScheduling() {
    // Create test campaign data
    QVariantMap campaignData = createTestCampaign("test_campaign_001");

    // Schedule campaign through AdService
    bool success = mAdService->scheduleCampaign(campaignData);
    QVERIFY(success);

    // Verify campaign was scheduled
    QVariantMap scheduledCampaign = mAdService->getCampaign("test_campaign_001");
    QVERIFY(!scheduledCampaign.isEmpty());
    QCOMPARE(scheduledCampaign["status"].toString(), QString("scheduled"));

    // Verify campaign appears in active campaigns
    QList<QVariantMap> activeCampaigns = mAdService->getActiveCampaigns();
    QVERIFY(activeCampaigns.size() > 0);
    bool found = false;
    for (const auto& campaign : activeCampaigns) {
        if (campaign["campaignId"].toString() == "test_campaign_001") {
            found = true;
            break;
        }
    }
    QVERIFY(found);
}

void AdPluginTest::testPaymentProcessing() {
    // Create test payment data
    QVariantMap paymentData = createTestPaymentData();

    // Create payment through PaymentFactory
    auto payment = mPaymentFactory->createPayment("ad");
    QVERIFY(payment != nullptr);

    // Configure payment
    payment->setParameters(paymentData);
    QVariantMap params = payment->getParameters();
    QCOMPARE(params["amount"].toInt(), 1000);
    QCOMPARE(params["adId"].toString(), QString("test_banner"));

    // Process payment
    bool success = payment->process();
    QVERIFY(success);

    // Verify payment status
    QVariantMap status = payment->getStatus();
    QCOMPARE(status["status"].toString(), QString("completed"));
    QVERIFY(status.contains("transactionId"));

    // Clean up
    mPaymentFactory->releasePayment(payment);
}
```text

### Test Execution

```bash
# Run Ad plugin tests specifically
cmake --build build/win-msvc-qt5-x64 --target ad_plugin_test

# Run tests with detailed output
ctest --output-on-failure --verbose -R ad_plugin

# Run specific test cases
ctest --output-on-failure -R "AdPluginTest.*Campaign"

# Run with memory checking (Valgrind)
valgrind --leak-check=full cmake --build build/linux-gcc-qt5-x64 --target ad_plugin_test

# Generate test coverage report
cmake --build build/linux-gcc-qt5-x64 --target coverage
```text

### Test Data Management

```cpp
// Test data helper functions
QVariantMap createTestCampaign(const QString& campaignId) {
    QVariantMap campaign;
    campaign["campaignId"] = campaignId;
    campaign["contentId"] = "test_banner_001";
    campaign["startDate"] = QDateTime::currentDateTime();
    campaign["endDate"] = QDateTime::currentDateTime().addDays(7);
    campaign["priority"] = 1;
    campaign["maxDisplays"] = 100;
    campaign["targetAudience"] = "test_group";
    return campaign;
}

QVariantMap createTestPaymentData() {
    QVariantMap payment;
    payment["paymentType"] = "ad";
    payment["amount"] = 1000;
    payment["currency"] = "UZS";
    payment["adId"] = "test_banner_001";
    payment["campaignId"] = "test_campaign_001";
    payment["merchantId"] = "TEST_MERCHANT";
    payment["terminalId"] = "TEST_TERMINAL";
    return payment;
}

// Test data cleanup
void cleanupTestData() {
    // Remove test campaigns
    QList<QVariantMap> campaigns = mAdService->getAllCampaigns();
    for (const auto& campaign : campaigns) {
        if (campaign["campaignId"].toString().startsWith("test_")) {
            mAdService->cancelCampaign(campaign["campaignId"].toString());
        }
    }

    // Remove test content
    QList<QString> content = mAdService->listAvailableContent();
    for (const auto& contentId : content) {
        if (contentId.startsWith("test_")) {
            mAdService->removeContent(contentId);
        }
    }
}
```text

---

## Dependencies

### Internal Dependency Analysis

| Dependency             | Purpose                                        | Version Requirements | Criticality |
| ---------------------- | ---------------------------------------------- | -------------------- | ----------- |
| **BasicApplication**   | Core application framework and utilities       | 1.0+                 | ✅ Critical |
| **Connection**         | Network connection management and utilities    | 1.0+                 | ✅ Critical |
| **PluginsSDK**         | Plugin system infrastructure and interfaces    | 1.0+                 | ✅ Critical |
| **PaymentBase**        | Payment processing base classes and interfaces | 1.0+                 | ✅ Critical |
| **NetworkTaskManager** | Network task management and utilities          | 1.0+                 | ✅ Critical |
| **SysUtils**           | System utilities and helpers                   | 1.0+                 | ✅ Critical |
| **ek_boost**           | Boost libraries wrapper and utilities          | 1.0+                 | ✅ Critical |

### External Dependency Analysis

| Dependency     | Purpose                                  | Version Requirements | Platform Notes |
| -------------- | ---------------------------------------- | -------------------- | -------------- |
| **Qt Core**    | Core Qt functionality                    | Qt5.6+ or Qt6.0+     | Cross-platform |
| **Qt Network** | Network functionality for remote updates | Qt5.6+ or Qt6.0+     | Cross-platform |
| **Qt Test**    | Testing framework (development only)     | Qt5.6+ or Qt6.0+     | Cross-platform |

### Platform-Specific Dependencies

| Platform    | Dependency         | Purpose               | Notes                        |
| ----------- | ------------------ | --------------------- | ---------------------------- |
| **Windows** | WinHTTP            | HTTP communications   | Optional, fallback available |
| **Linux**   | libcurl            | HTTP communications   | Recommended for production   |
| **macOS**   | Security.framework | Secure communications | Required for HTTPS           |
| **All**     | OpenSSL            | Secure communications | Required for HTTPS support   |
| **All**     | SQLite             | Local database        | Optional, used for caching   |

### Dependency Resolution

```cmake
# CMake dependency resolution example
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

if(WIN32)
    find_package(Boost REQUIRED COMPONENTS system filesystem)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

if(UNIX AND NOT APPLE)
    find_package(CURL REQUIRED)
    add_definitions(-DUSE_LIBCURL)
endif()

# EKiosk internal dependencies
add_dependencies(ad_plugin
    BasicApplication
    Connection
    PluginsSDK
    PaymentBase
    NetworkTaskManager
    SysUtils
)
```text

---

## Troubleshooting

### Comprehensive Issue Resolution Guide

#### Plugin Initialization Issues

**Symptom**: Plugin fails to initialize during application startup
**Root Causes**:

- Missing or incompatible dependencies
- Incorrect plugin metadata
- Configuration file errors
- Permission issues
- Resource conflicts

**Diagnostic Steps**:

```bash
# Verify plugin file integrity
checksum plugins/libad_plugin.so

# Check dependency availability
ldd plugins/libad_plugin.so

# Validate plugin metadata
validate_plugin_metadata ad_plugin

# Check configuration file syntax
check_plugin_config ad_plugin
```text

**Resolution Matrix**:

| Issue                | Detection                               | Resolution                     | Prevention                         |
| -------------------- | --------------------------------------- | ------------------------------ | ---------------------------------- |
| Missing dependencies | `ldd` shows missing libraries           | Install missing packages       | Document dependencies clearly      |
| Version mismatch     | Plugin fails to load with version error | Update to compatible versions  | Use dependency management          |
| Configuration errors | Plugin logs configuration parse errors  | Fix configuration syntax       | Validate configuration files       |
| Permission issues    | Plugin fails with access denied         | Set correct permissions        | Use proper installation procedures |
| Resource conflicts   | Plugin crashes on startup               | Identify and resolve conflicts | Test in isolated environments      |

#### Campaign Management Problems

**Symptom**: Campaigns not displaying or behaving incorrectly
**Root Causes**:

- Invalid campaign configuration
- Content not available
- Scheduling conflicts
- Priority issues
- Database corruption

**Diagnostic Commands**:

```cpp
// Check campaign status
QVariantMap campaignStatus = adService->getCampaignDiagnostics("campaign_id");

// Validate campaign configuration
bool isValid = adService->validateCampaign(campaignData);

// Check content availability
QVariantMap contentStatus = adService->checkContentAvailability("content_id");

// Verify database integrity
bool dbOk = adService->verifyDatabaseIntegrity();
```text

**Common Solutions**:

```cpp
// Fix invalid campaign configuration
QVariantMap fixedCampaign = adService->normalizeCampaign(campaignData);
bool success = adService->updateCampaign("campaign_id", fixedCampaign);

// Resolve content issues
if (!contentAvailable) {
    bool contentFixed = adService->repairContent("content_id");
    if (!contentFixed) {
        // Re-download content
        bool downloaded = adService->downloadContent("content_id");
    }
}

// Handle database corruption
if (!dbOk) {
    bool repaired = adService->repairDatabase();
    if (!repaired) {
        // Restore from backup
        bool restored = adService->restoreFromBackup();
    }
}
```text

#### Payment Processing Failures

**Symptom**: Advertisement payments not processing correctly
**Root Causes**:

- Invalid payment parameters
- Payment service unavailable
- Network connectivity issues
- Transaction conflicts
- Authorization failures

**Payment Debugging**:

```cpp
// Enable payment debugging
QVariantMap debugConfig;
debugConfig["paymentDebug"] = true;
debugConfig["logTransactions"] = true;
adPlugin->setConfiguration(debugConfig);

// Test payment processing step-by-step
auto payment = paymentFactory->createPayment("ad");
payment->setParameters(paymentData);

// Validate payment before processing
QVariantMap validation = payment->validate();
if (validation["valid"].toBool()) {
    bool success = payment->process();
    if (!success) {
        QVariantMap error = payment->getLastError();
        // Handle specific error
    }
}
```text

**Payment Recovery**:

```cpp
// Handle failed payments
QList<QVariantMap> failedPayments = paymentFactory->getFailedPayments();
for (const auto& failedPayment : failedPayments) {
    QString transactionId = failedPayment["transactionId"].toString();
    bool recovered = paymentFactory->recoverPayment(transactionId);
    if (!recovered) {
        // Manual intervention required
        logFailedPayment(failedPayment);
    }
}

// Retry payments
bool retrySuccess = paymentFactory->retryFailedPayments();
```text

#### Remote Update Issues

**Symptom**: Remote content updates failing
**Root Causes**:

- Network connectivity problems
- Server unavailable
- Authentication failures
- Content format errors
- Disk space issues

**Update Diagnostics**:

```bash
# Network diagnostics
ping ad-server.example.com
traceroute ad-server.example.com
nslookup ad-server.example.com

# Server status
curl -I https://ad-server.example.com/status

# Content validation
validate_ad_content downloaded_content.json
```text

**Update Recovery**:

```cpp
// Manual update trigger
QVariantMap manualUpdate;
manualUpdate["forceUpdate"] = true;
manualUpdate["retryFailed"] = true;
bool success = adService->triggerManualUpdate(manualUpdate);

// Partial update recovery
QVariantMap partialUpdate;
partialUpdate["contentTypes"] = QStringList() << "banners";
partialUpdate["ignoreErrors"] = true;
bool partialSuccess = adService->partialUpdate(partialUpdate);

// Content repair
bool contentRepaired = adService->repairUpdatedContent();
```text

### Advanced Debugging Techniques

**Performance Profiling**:

```bash
# CPU profiling
perf record -g -p $(pidof ekiosk)
perf report

# Memory profiling
valgrind --tool=massif ekiosk
ms_print massif.out.*

# I/O profiling
strace -p $(pidof ekiosk) -e trace=file
```text

**Logging Configuration**:

```cpp
// Configure detailed logging
QVariantMap loggingConfig;
loggingConfig["logLevel"] = "debug";
loggingConfig["logComponents"] = QStringList() << "AdPlugin" << "AdService" << "PaymentFactory";
loggingConfig["logToFile"] = true;
loggingConfig["logToConsole"] = true;
loggingConfig["maxLogSize"] = 1024 * 1024 * 10; // 10MB
adPlugin->setConfiguration(loggingConfig);

// Log analysis
analyze_ad_logs /var/log/ekiosk/ad_plugin.log
```text

**Memory Analysis**:

```cpp
// Memory leak detection
QVariantMap memoryConfig;
memoryConfig["trackMemory"] = true;
memoryConfig["leakDetection"] = true;
adPlugin->setConfiguration(memoryConfig);

// Memory usage monitoring
connect(adService, &AdService::memoryUsageUpdated,
        this, &DebugHelper::logMemoryUsage);

// Force garbage collection
adService->forceGarbageCollection();
```text

---

## Migration notes

### Version Compatibility Matrix

| Version | Qt5 Support | Qt6 Support | API Stability | Notes                         |
| ------- | ----------- | ----------- | ------------- | ----------------------------- |
| 1.0     | ✅ Full     | ✅ Full     | ✅ Stable     | Current production version    |
| 0.9     | ✅ Full     | ❌ None     | ⚠️ Beta       | Beta with payment integration |
| 0.5-0.8 | ✅ Partial  | ❌ None     | ❌ Unstable   | Development versions          |
| 0.1-0.4 | ✅ Basic    | ❌ None     | ❌ Unstable   | Early prototypes              |

### API Evolution Timeline

**1.0 (Current)**:

- Stable API with comprehensive functionality
- Full Qt5/Qt6 compatibility
- Complete documentation
- Production-ready

**0.9 (Beta)**:

- Added payment processing integration
- Improved campaign management
- Basic statistics tracking
- Limited error handling

**0.5-0.8 (Development)**:

- Experimental API changes
- Incomplete functionality
- Limited testing
- Not recommended for production

### Migration Paths

**From 0.9 to 1.0**:

```bash
# No breaking changes
# Configuration format unchanged
# API fully compatible
# Recommended upgrade for all users

# Migration steps:
1. Backup existing configuration
2. Update plugin files
3. Verify dependencies
4. Test functionality
5. Deploy to production
```text

**From Legacy Systems**:

```bash
# Significant changes required
# API redesign and modernization
# Configuration format changes
# Comprehensive testing required

# Migration steps:
1. Analyze legacy implementation
2. Map legacy API to new API
3. Update configuration files
4. Test migration in staging
5. Gradual production rollout
6. Monitor and optimize
```text

### Breaking Changes History

**No breaking changes in current version (1.0)**

**Potential Future Breaking Changes (2.0)**:

- API cleanup and modernization
- Configuration format changes
- Dependency updates
- Architecture improvements

All breaking changes will be:

1. Documented in migration guides
2. Announced well in advance
3. Provided with migration tools
4. Supported with backward compatibility where possible

---

## Further reading

### Core Documentation

- [Plugin System Architecture](README.md#plugin-architecture)
- [EKiosk Developer Guide](../../docs/getting-started.md)
- [Testing Guide](../../docs/testing.md)
- [CMake Build System](../../docs/build-guide.md)

### Related Services

- [Ad Service Documentation](../../docs/services/ad.md)
- [Payment Service Documentation](../../docs/services/payment.md)
- [Database Service Documentation](../../docs/services/database.md)
- [Network Service Documentation](../../docs/services/network.md)

### External Resources

- [Qt Plugin System](https://doc.qt.io/qt-6/plugins-howto.html)
- [Qt Payment Processing](https://doc.qt.io/qt-6/qml-qtquick-controls-styles-payment.html)
- [Advertisement Standards](https://www.iab.com/standards/)

### Source Code Reference

- **Main Plugin**: `src/plugins/Ad/`
- **Tests**: `tests/plugins/Ad/`
- **Documentation**: `docs/plugins/ad.md`
- **Examples**: `examples/ad_plugin/`

---

## Configuration Reference

### Complete Configuration Specification

```ini
[AdPlugin]
; =============================================
; DISPLAY AND ROTATION SETTINGS
; =============================================
; Interval between advertisement displays in seconds
displayInterval=300

; Maximum number of concurrent active campaigns
maxCampaigns=10

; Enable automatic campaign rotation
autoRotate=false

; Default campaign to display when no others are active
defaultCampaign=welcome_screen

; Rotation strategy: "priority", "round-robin", "random"
rotationStrategy=priority

; =============================================
; CONTENT MANAGEMENT
; =============================================
; Path to advertisement content storage
contentPath=/opt/ekiosk/ads

; Maximum content cache size in MB
maxContentCache=500

; Content validation level: "basic", "strict", "none"
contentValidation=strict

; Enable content compression
enableCompression=true

; =============================================
; REMOTE UPDATE SETTINGS
; =============================================
; Enable remote content updates
enableRemoteUpdates=true

; Remote update check interval in seconds
updateInterval=3600

; Remote advertisement server URL
remoteServer=https://ad-server.example.com/api/v1

; Update strategy: "auto", "manual", "scheduled"
updateStrategy=auto

; Maximum download speed in KB/s (0 = unlimited)
maxDownloadSpeed=0

; =============================================
; STATISTICS AND REPORTING
; =============================================
; Enable statistics tracking
statisticsEnabled=true

; Statistics retention period in days
statsRetention=30

; Enable real-time reporting
realTimeReporting=false

; Reporting interval in minutes
reportingInterval=60

; =============================================
; PERFORMANCE AND DEBUGGING
; =============================================
; Enable debug mode
debugMode=false

; Log level: "error", "warning", "info", "debug"
logLevel=info

; Log file path
logFile=/var/log/ekiosk/ad_plugin.log

; Maximum log file size in MB
maxLogSize=10

; Enable performance monitoring
performanceMonitoring=false

; Memory usage threshold for warnings (MB)
memoryWarningThreshold=256

; =============================================
; ADVANCED SETTINGS
; =============================================
; Database connection string
databaseConnection=sqlite:///var/ekiosk/ad_plugin.db

; Content encryption key (leave empty to disable)
encryptionKey=

; Enable content preloading
enablePreloading=true

; Preload threshold in seconds
preloadThreshold=30

; Enable fallback content
enableFallback=true

; Fallback content ID
fallbackContent=default_banner
```text

### Configuration Management API

```cpp
// Comprehensive configuration management
class AdPluginConfigManager {
public:
    // Load configuration from multiple sources
    bool loadConfiguration();
    bool loadFromFile(const QString& filePath);
    bool loadFromDatabase();
    bool loadFromEnvironment();

    // Save configuration
    bool saveConfiguration();
    bool saveToFile(const QString& filePath);
    bool saveToDatabase();

    // Configuration validation
    bool validateConfiguration();
    QVariantMap getValidationErrors();
    bool fixConfigurationErrors();

    // Configuration merging
    bool mergeConfiguration(const QVariantMap& newConfig);
    bool mergeFromFile(const QString& filePath);

    // Configuration export/import
    bool exportConfiguration(const QString& format, QByteArray& output);
    bool importConfiguration(const QString& format, const QByteArray& input);

    // Configuration monitoring
    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const;

    // Configuration events
    void onConfigurationChanged(std::function<void(const QVariantMap&)> callback);
    void onValidationError(std::function<void(const QString&)> callback);
};

// Usage examples
AdPluginConfigManager configManager;

// Load and validate configuration
bool loaded = configManager.loadConfiguration();
if (loaded) {
    bool valid = configManager.validateConfiguration();
    if (!valid) {
        QVariantMap errors = configManager.getValidationErrors();
        bool fixed = configManager.fixConfigurationErrors();
    }
}

// Monitor configuration changes
configManager.onConfigurationChanged([](const QVariantMap& newConfig) {
    qDebug() << "Configuration changed:" << newConfig;
});

configManager.startMonitoring();
```text

---

## Best Practices

### Plugin Development Best Practices

**Architecture**:

- Follow single responsibility principle
- Use clear separation of concerns
- Implement proper error handling
- Design for testability

**Code Quality**:

- Write comprehensive unit tests
- Maintain 100% test coverage
- Use meaningful variable names
- Follow consistent coding style

**Documentation**:

- Keep README updated
- Document all public APIs
- Provide usage examples
- Maintain change logs

**Performance**:

- Optimize critical paths
- Minimize memory usage
- Use efficient data structures
- Implement proper caching

### Plugin Usage Best Practices

**Initialization**:

- Always check plugin readiness
- Handle initialization errors gracefully
- Verify dependencies are available
- Load configuration before use

**Configuration**:

- Use default configuration as baseline
- Validate configuration changes
- Save configuration after modifications
- Handle configuration errors

**Error Handling**:

- Implement comprehensive error handling
- Provide meaningful error messages
- Log errors for debugging
- Implement recovery mechanisms

**Resource Management**:

- Properly release resources
- Handle memory efficiently
- Clean up on plugin shutdown
- Monitor resource usage

### Deployment Best Practices

**Configuration**:

- Provide default configuration files
- Document configuration options
- Validate configurations before deployment
- Use environment-specific configurations

**Dependencies**:

- Document all dependencies
- Include required dependencies
- Verify dependency versions
- Test with dependency updates

**Testing**:

- Test on target platforms
- Verify all functionality
- Test error conditions
- Perform load testing

**Monitoring**:

- Implement health monitoring
- Set up alerting
- Monitor performance metrics
- Track usage statistics

---

## Support

### Support Resources

**Official Support Channels**:

- **GitHub Issues**: <https://github.com/ekiosk/ekiosk/issues>
- **Documentation**: <https://ekiosk.docs.com>
- **Community Forum**: <https://community.ekiosk.com>
- **Professional Support**: <support@ekiosk.com>

**Support Levels**:

- **Community**: Free support through GitHub and forums
- **Standard**: Business hours support with SLA
- **Premium**: 24/7 support with priority response
- **Enterprise**: Dedicated support engineer

### Troubleshooting Workflow

1. **Identify Issue**: Gather symptoms and error messages
2. **Reproduce Issue**: Create reproducible test case
3. **Check Documentation**: Review relevant documentation
4. **Search Knowledge Base**: Look for similar issues
5. **Diagnose**: Use debugging tools and techniques
6. **Resolve**: Implement fix or workaround
7. **Test**: Verify resolution
8. **Document**: Update documentation if needed

### Support Request Template

```markdown
## Support Request

**Plugin**: Ad Plugin
**Version**: 1.0
**Platform**: [Windows/Linux/macOS]
**Qt Version**: [5.x/6.x]

### Issue Description

[Detailed description of the issue]

### Steps to Reproduce

1. [Step 1]
2. [Step 2]
3. [Step 3]

### Expected Behavior

[What should happen]

### Actual Behavior

[What actually happens]

### Error Messages
```text

[Copy error messages here]

````

### Configuration
```ini
[Relevant configuration sections]
````

### Environment

- **OS Version**: [e.g., Windows 10 21H2]
- **Hardware**: [CPU, RAM, etc.]
- **Dependencies**: [List relevant dependencies and versions]

### Additional Information

[Any other relevant information]

---

## License

### License Information

The Ad Plugin is licensed under the **EKiosk Commercial License**, which grants the following rights:

- **Usage**: Free for personal and commercial use
- **Modification**: Allowed with attribution
- **Distribution**: Allowed in binary form
- **Support**: Optional professional support available

### License Compliance

```cpp
// Include license information in plugin metadata
QString SDK::Plugin::PluginFactory::mLicense = "EKiosk Commercial License";
QString SDK::Plugin::PluginFactory::mLicenseUrl = "https://ekiosk.com/license";

// Display license information
void showLicenseInformation() {
    QMessageBox::information(
        nullptr,
        "License Information",
        "Ad Plugin\n"
        "Version: " + SDK::Plugin::PluginFactory::mVersion + "\n"
        "License: " + SDK::Plugin::PluginFactory::mLicense + "\n"
        "Copyright © " + QDate::currentDate().year() + " EKiosk Technologies"
    );
}
````

### Third-Party Licenses

The Ad Plugin includes the following third-party components:

| Component       | License       | Version | Usage                 |
| --------------- | ------------- | ------- | --------------------- |
| Qt Framework    | LGPL v3       | 5.x/6.x | Core functionality    |
| Boost Libraries | BSL 1.0       | 1.7x    | Utility functions     |
| SQLite          | Public Domain | 3.x     | Database storage      |
| OpenSSL         | Apache 2.0    | 1.x/3.x | Secure communications |

### License Verification

```bash
# Verify license compliance
check_license_compliance ad_plugin

# Generate license report
generate_license_report --output ad_plugin_licenses.txt

# Check third-party licenses
check_third_party_licenses ad_plugin
```text

---

## Appendix

### Glossary

| Term                  | Definition                                                          |
| --------------------- | ------------------------------------------------------------------- |
| **Campaign**          | A collection of advertisements with common targeting and scheduling |
| **Content**           | The actual advertisement material (images, videos, etc.)            |
| **Impression**        | A single display of an advertisement                                |
| **Click-through**     | User interaction with an advertisement                              |
| **CTR**               | Click-through rate (clicks/impressions)                             |
| **Sponsored Content** | Advertisement content that sponsors a payment transaction           |
| **Rotation**          | The process of cycling through multiple advertisements              |
| **Priority**          | The relative importance of a campaign in display selection          |

### Acronyms

| Acronym | Expansion                             |
| ------- | ------------------------------------- |
| CTR     | Click-Through Rate                    |
| CPC     | Cost Per Click                        |
| CPM     | Cost Per Mille (thousand impressions) |
| ROI     | Return On Investment                  |
| KPI     | Key Performance Indicator             |
| API     | Application Programming Interface     |
| SDK     | Software Development Kit              |
| HTTPS   | Hypertext Transfer Protocol Secure    |
| JSON    | JavaScript Object Notation            |

### References

- **IAB Standards**: <https://www.iab.com/standards/>
- **Qt Documentation**: <https://doc.qt.io/>
- **Payment Processing**: <https://www.pcisecuritystandards.org/>
- **Advertisement Metrics**: <https://www.iab.com/guidelines/>

### Change Log

**Version 1.0**:

- Initial stable release
- Complete documentation
- Production-ready
- Full feature set

**Version 0.9**:

- Beta release
- Payment integration
- Basic statistics
- Limited testing

**Version 0.5-0.8**:

- Development versions
- Experimental features
- API changes
- Not production-ready

### Contributors

- **Core Development**: EKiosk Development Team
- **Documentation**: Technical Writing Team
- **Testing**: QA Engineering Team
- **Support**: Customer Support Team

### Acknowledgements

Special thanks to:

- Early adopters and beta testers
- Community contributors
- Open source contributors
- Documentation reviewers
