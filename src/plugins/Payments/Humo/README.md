# Humo Payments Plugin

Payment processing plugin for HUMO card transactions in EKiosk - provides comprehensive HUMO payment system integration with multi-stage payments, PIN management, and dealer operations.

## Purpose

The Humo Payments Plugin serves as the core payment processing system for HUMO card transactions in EKiosk, providing:

- **HUMO Card Processing**: Complete support for HUMO card payment transactions
- **Multi-stage Payments**: Complex payment flows with multiple sequential stages
- **PIN Management**: Secure PIN entry, validation, and management operations
- **Dealer Operations**: Support for dealer-specific payment processing and data management
- **Local Data Management**: Handling of local payment data and offline transactions
- **Payment Security**: Secure payment processing with encryption and validation

This plugin enables complete integration with the HUMO payment system for kiosk devices, supporting both online and offline payment scenarios.

---

## Quick start 🔧

```cpp
// Access Humo Payments plugin through the plugin system
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/IEnvironment.h>

// Get Humo Payments plugin instance
SDK::Plugin::IPlugin *humoPlugin = environment->getPlugin("humo_payments");
if (humoPlugin && humoPlugin->isReady()) {
    // Access PaymentFactory service
    auto paymentFactory = environment->getInterface<PaymentFactory>("HumoPayments");
    if (paymentFactory) {
        // Create a multistage payment
        QVariantMap paymentData;
        paymentData["amount"] = 50000;
        paymentData["currency"] = "UZS";
        paymentData["paymentType"] = "multistage";
        paymentData["dealerId"] = "DEALER123";

        auto payment = paymentFactory->createPayment("multistage");
        if (payment) {
            payment->setParameters(paymentData);
            bool success = payment->process();
            // Handle payment result...
        }
    }
}
```

---

## Features

### Core Payment Modules

- **Payment Factory**: Processes HUMO card payment transactions
  - Multi-stage payment processing
  - Dealer payment handling
  - PIN-based operations
  - Transaction management and validation

- **Card Management**: HUMO card operations and validation
  - Card reading and verification
  - Card balance inquiries
  - Transaction history retrieval
  - Card security operations

- **PIN Services**: PIN management and validation
  - PIN entry and verification
  - PIN change operations
  - PIN reset and recovery
  - Secure PIN handling

- **Dealer Services**: Dealer-specific functionality
  - Dealer identification and validation
  - Dealer transaction processing
  - Commission and fee management
  - Dealer data synchronization

- **Local Data Management**: Local payment data handling
  - Offline transaction storage
  - Local data synchronization
  - Data backup and recovery
  - Transaction queuing

### Service Components

- **HumoPaymentsFactory**: Qt plugin factory for registration and instantiation
- **PaymentFactory**: Core payment factory for creating payment instances
- **DealerPayment**: Dealer-specific payment processing
- **MultistagePayment**: Multi-stage payment flow implementation
- **PinPayment**: PIN management and validation
- **CardReaderService**: Card reading and validation service

### Integration Points

- **Payment System**: Integration with EKiosk payment processor
- **Card Reader Hardware**: Physical card reader communication
- **Database Service**: Transaction and payment data storage
- **Network Service**: Communication with HUMO payment servers
- **Security Service**: Encryption and security management
- **Logging Service**: Comprehensive transaction logging

---

## Platform support

### Qt Version Compatibility

**Qt Version Support:**

- ✅ Qt5 compatible
- ✅ Qt6 compatible
- No version-specific requirements

### Platform Support Table

- **Windows**: ✅ Full - Complete HUMO payment functionality
  - Full card processing support
  - Multi-stage payment flows
  - PIN management operations
  - Dealer data processing
  - Native Windows integration
  - PC/SC smart card support

- **Linux**: ✅ Full - Complete functionality with platform-specific features
  - Full card processing
  - Multi-stage payments
  - PIN operations
  - Dealer functionality
  - PC/SC smart card support
  - Systemd service integration

- **macOS**: 🔬 TODO - Limited testing, core functionality should work
  - Basic functionality tested
  - Requires additional testing for production use
  - May need platform-specific adjustments
  - PC/SC smart card support

---

## Configuration

The Humo Payments plugin supports comprehensive runtime configuration:

```cpp
// Get current configuration
QVariantMap config = humoPlugin->getConfiguration();

// Update configuration options
config["dealerId"] = "DEALER123";
config["timeout"] = 30000; // milliseconds
config["retryAttempts"] = 3;
config["enableLogging"] = true;
config["offlineMode"] = false;

// Apply configuration
humoPlugin->setConfiguration(config);

// Save configuration permanently
humoPlugin->saveConfiguration();
```

### Configuration Options Reference

| Option                 | Type   | Default | Description                                      | Example Values           |
| ---------------------- | ------ | ------- | ------------------------------------------------ | ------------------------ |
| `dealerId`             | string | ""      | Dealer identifier for transactions               | "DEALER123", "DEALER456" |
| `timeout`              | int    | 30000   | Transaction timeout in milliseconds              | 15000, 30000, 60000      |
| `retryAttempts`        | int    | 3       | Number of retry attempts for failed transactions | 1, 3, 5                  |
| `enableLogging`        | bool   | true    | Enable transaction logging                       | true, false              |
| `offlineMode`          | bool   | false   | Enable offline transaction processing            | true, false              |
| `maxTransactionAmount` | int    | 1000000 | Maximum transaction amount                       | 500000, 1000000, 5000000 |
| `currency`             | string | "UZS"   | Default currency for transactions                | "UZS", "USD"             |
| `pinRetries`           | int    | 3       | Maximum PIN entry attempts                       | 3, 5                     |

### Configuration File

Configuration is stored in the plugin's configuration file:

```
[HumoPaymentsPlugin]
dealerId=DEALER123
timeout=30000
retryAttempts=3
maxTransactionAmount=1000000
currency=UZS
pinRetries=3
enableLogging=true
offlineMode=false
```

---

## Usage / API highlights

### Main Humo Payments Plugin Operations

```cpp
// Show Humo Payments plugin
humoPlugin->show();

// Hide Humo Payments plugin
humoPlugin->hide();

// Reset Humo Payments plugin state
humoPlugin->reset(QVariantMap());

// Check if Humo Payments plugin is ready
bool isReady = humoPlugin->isReady();

// Get Humo Payments plugin error status
QString error = humoPlugin->getError();
```

### Dealer Payment API

```cpp
// Access PaymentFactory
auto paymentFactory = environment->getInterface<PaymentFactory>("HumoPayments");
if (paymentFactory) {
    // Create dealer payment
    QVariantMap dealerPaymentData;
    dealerPaymentData["dealerId"] = "DEALER123";
    dealerPaymentData["amount"] = 100000;
    dealerPaymentData["currency"] = "UZS";
    dealerPaymentData["transactionId"] = generateTransactionId();
    dealerPaymentData["description"] = "Service payment";

    auto dealerPayment = paymentFactory->createPayment("dealer");
    if (dealerPayment) {
        dealerPayment->setParameters(dealerPaymentData);
        bool success = dealerPayment->process();

        if (success) {
            QVariantMap result = dealerPayment->getResult();
            QString transactionId = result["transactionId"].toString();
            // Handle successful dealer payment
        }
    }
}
```

### Multi-stage Payment API

```cpp
// Create multistage payment
QVariantMap multistageData;
multistageData["amount"] = 250000;
multistageData["currency"] = "UZS";
multistageData["stages"] = QVariantList() << "authorization" << "capture" << "completion";
multistageData["stageTimeout"] = 10000; // 10 seconds per stage

auto multistagePayment = paymentFactory->createPayment("multistage");
if (multistagePayment) {
    multistagePayment->setParameters(multistageData);

    // Process each stage
    bool allStagesSuccessful = true;
    QVariantList stages = multistageData["stages"].toList();

    for (const QVariant& stage : stages) {
        bool stageSuccess = multistagePayment->processStage(stage.toString());
        if (!stageSuccess) {
            allStagesSuccessful = false;
            // Handle stage failure, possibly rollback
            multistagePayment->rollbackToStage(stage.toString());
            break;
        }
    }

    if (allStagesSuccessful) {
        // All stages completed successfully
        QVariantMap finalResult = multistagePayment->getFinalResult();
    }
}
```

### PIN Management API

```cpp
// Create PIN payment for PIN operations
QVariantMap pinData;
pinData["operation"] = "validate"; // "validate", "change", "reset"
pinData["cardNumber"] = "8600123456789012";
pinData["currentPin"] = "1234"; // for validation/change operations
pinData["newPin"] = "5678"; // for change operations

auto pinPayment = paymentFactory->createPayment("pin");
if (pinPayment) {
    pinPayment->setParameters(pinData);
    bool success = pinPayment->process();

    if (success) {
        QVariantMap result = pinPayment->getResult();
        bool pinValid = result["pinValid"].toBool();
        QString status = result["status"].toString();
        // Handle PIN operation result
    }
}
```

### Card Operations API

```cpp
// Access card reader service
auto cardReader = environment->getInterface<CardReaderService>("CardReader");
if (cardReader) {
    // Read card information
    QVariantMap cardInfo = cardReader->readCard();
    QString cardNumber = cardInfo["cardNumber"].toString();
    QString cardType = cardInfo["cardType"].toString();

    // Check card balance
    QVariantMap balanceData;
    balanceData["cardNumber"] = cardNumber;
    QVariantMap balance = cardReader->getBalance(balanceData);
    double availableBalance = balance["availableBalance"].toDouble();

    // Get transaction history
    QVariantMap historyParams;
    historyParams["cardNumber"] = cardNumber;
    historyParams["limit"] = 10;
    QVariantList transactions = cardReader->getTransactionHistory(historyParams);
}
```

---

## Integration

### CMake Configuration

The Humo Payments plugin uses the standard EKiosk plugin CMake configuration:

```cmake
# From src/plugins/Payments/Humo/CMakeLists.txt
include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

set(HUMO_PAYMENTS_PLUGIN_SOURCES
    src/HumoPaymentsFactory.cpp
    src/HumoPaymentsFactory.h
    src/HumoPaymentsPluginImpl.cpp
    src/HumoPaymentsPluginImpl.h
    src/PaymentFactory.cpp
    src/PaymentFactory.h
    src/DealerPayment.cpp
    src/DealerPayment.h
    src/MultistagePayment.cpp
    src/MultistagePayment.h
    src/PinPayment.cpp
    src/PinPayment.h
    src/HumoPaymentBase.cpp
    src/HumoPaymentBase.h
    src/CardReaderService.cpp
    src/CardReaderService.h
    src/CardOperations.cpp
    src/CardOperations.h
)

ek_add_plugin(humo_payments
    FOLDER "plugins"
    SOURCES ${HUMO_PAYMENTS_PLUGIN_SOURCES}
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
# Configure and build the entire project (includes Humo Payments plugin)
cmake --preset win-msvc-qt5-x64
cmake --build build/win-msvc-qt5-x64 --target humo_payments

# Build just the Humo Payments plugin
cmake --build build/win-msvc-qt5-x64 --target humo_payments
```

### Plugin Loading

The Humo Payments plugin is automatically loaded by the EKiosk plugin system:

```cpp
// Plugin is loaded automatically during application startup
// Access through the plugin system:
SDK::Plugin::IPlugin *humoPlugin = environment->getPlugin("humo_payments");

// Access through payment factory:
auto paymentFactory = environment->getInterface<PaymentFactory>("HumoPayments");
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

The Humo Payments plugin includes comprehensive tests using the EKiosk testing framework with mock kernel infrastructure.

### Test Coverage Areas

- **Plugin Loading**: Factory registration and plugin instantiation
- **Initialization**: Plugin startup and dependency injection
- **Dealer Payments**: Dealer payment processing and validation
- **Multi-stage Payments**: Complex payment flow processing
- **PIN Operations**: PIN validation and management
- **Card Operations**: Card reading and validation
- **Offline Processing**: Offline transaction handling
- **Error Handling**: Graceful failure and recovery

### Test Structure

```text
tests/plugins/Payments/Humo/
├── humo_payments_plugin_test.cpp     # Main test file
├── mock_payment_factory.h/.cpp       # Mock PaymentFactory implementation
├── mock_card_reader.h/.cpp          # Mock CardReaderService implementation
├── test_dealer_payments/            # Test dealer payment data
├── test_multistage_payments/        # Test multistage payment data
├── test_pin_operations/             # Test PIN operation data
├── test_configs/                    # Test configuration files
└── CMakeLists.txt                   # Test build configuration
```

### Running Tests

```bash
# Run Humo Payments plugin tests
cmake --build build/win-msvc-qt5-x64 --target humo_payments_test

# Run tests with verbose output
ctest --output-on-failure -R humo_payments

# Run all plugin tests
cmake --build build/win-msvc-qt5-x64 --target run_tests
```

### Test Examples

```cpp
#include "../common/PluginTestBase.h"
#include <SDK/Plugins/IPlugin.h>
#include <SDK/PaymentProcessor/Payment/IPaymentFactory.h>

class HumoPaymentsPluginTest : public QObject {
    Q_OBJECT

private slots:
    void testPluginLoading();
    void testPluginInitialization();
    void testDealerPaymentProcessing();
    void testMultistagePaymentProcessing();
    void testPinOperations();
    void testCardOperations();
    void testOfflineProcessing();
    void testErrorHandling();
};

void HumoPaymentsPluginTest::testPluginLoading() {
    // Test plugin factory and loading
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory("humo_payments");
    QVERIFY(factory != nullptr);
    QCOMPARE(factory->getName(), QString("Humo Payments Plugin"));
    QCOMPARE(factory->getModuleName(), QString("humo_payments"));
}

void HumoPaymentsPluginTest::testDealerPaymentProcessing() {
    // Test dealer payment processing
    auto paymentFactory = m_testBase.getInterface<PaymentFactory>("HumoPayments");
    QVERIFY(paymentFactory != nullptr);

    // Test payment creation and processing
    auto payment = paymentFactory->createPayment("dealer");
    QVERIFY(payment != nullptr);

    QVariantMap paymentData;
    paymentData["dealerId"] = "DEALER123";
    paymentData["amount"] = 100000;
    paymentData["currency"] = "UZS";

    payment->setParameters(paymentData);
    bool success = payment->process();
    QVERIFY(success);

    paymentFactory->releasePayment(payment);
}
```

### Test Coverage Requirements

- **100% Coverage**: All public methods must be tested
- **Error Paths**: Test failure scenarios and error handling
- **Integration**: Test plugin integration with payment and card services
- **Performance**: Test with realistic transaction volumes
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

| Dependency     | Purpose                                        | Version Requirements |
| -------------- | ---------------------------------------------- | -------------------- |
| **Qt Core**    | Core Qt functionality                          | Qt5 or Qt6           |
| **Qt Network** | Network functionality for server communication | Qt5 or Qt6           |
| **PC/SC**      | Smart card interface library                   | 1.x                  |

### Platform-Specific Dependencies

| Platform | Dependency       | Purpose                        |
| -------- | ---------------- | ------------------------------ |
| **All**  | Database Service | Transaction and payment data   |
| **All**  | Network Service  | HUMO server communication      |
| **All**  | Payment Service  | Payment processing integration |
| **All**  | Security Service | Transaction encryption         |
| **All**  | PC/SC Library    | Smart card communication       |

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
- PC/SC library issues

**Solutions**:

```bash
# Verify plugin file exists
ls plugins/humo_payments.dll  # Windows
ls plugins/libhumo_payments.so  # Linux

# Check dependencies
ldd plugins/libhumo_payments.so  # Linux
dumpbin /DEPENDENTS plugins/humo_payments.dll  # Windows

# Verify PC/SC library
check_pcsc_library

# Verify plugin configuration
check_plugin_config humo_payments
```

#### Payment Processing Issues

**Symptom**: Payments not processing correctly
**Possible Causes**:

- Invalid payment configuration
- Card reader hardware issues
- Network connectivity problems
- Dealer configuration errors
- PIN validation failures

**Solutions**:

```cpp
// Verify payment factory
auto paymentFactory = environment->getInterface<PaymentFactory>("HumoPayments");
if (!paymentFactory) {
    // Payment factory not available
    initializePaymentFactory();
}

// Check dealer configuration
QVariantMap dealerConfig;
dealerConfig["dealerId"] = "valid_dealer_id";
bool dealerValid = paymentFactory->validateDealer("DEALER123");

// Test card reader connectivity
auto cardReader = environment->getInterface<CardReaderService>("CardReader");
bool cardReaderOk = cardReader->testConnection();
```

#### PIN Management Issues

**Symptom**: PIN operations not working
**Possible Causes**:

- Invalid PIN format
- Card reader PIN pad issues
- PIN retry limit exceeded
- Security policy violations
- Hardware failures

**Solutions**:

```cpp
// Enable PIN debugging
QVariantMap debugConfig;
debugConfig["pinDebug"] = true;
humoPlugin->setConfiguration(debugConfig);

// Test PIN operations
auto pinPayment = paymentFactory->createPayment("pin");
QVariantMap pinData;
pinData["operation"] = "validate";
pinData["cardNumber"] = "8600123456789012";
pinData["pin"] = "1234";

pinPayment->setParameters(pinData);
bool success = pinPayment->process();
```

#### Card Reader Issues

**Symptom**: Card reader not functioning
**Possible Causes**:

- Hardware not connected
- Driver issues
- PC/SC service not running
- Permission problems
- Configuration errors

**Solutions**:

```bash
# Check PC/SC service status
systemctl status pcscd  # Linux
sc query "PC/SC Smart Card"  # Windows

# Test card reader hardware
pcsc_scan  # Linux

# Verify permissions
ls -la /dev/pcsc*  # Linux
cacls "\\\\.\\SCardSvrPipe"  # Windows

# Reset card reader service
cardReaderService->resetHardware();
```

### Debugging Techniques

**Enable Debug Mode**:

```cpp
QVariantMap config = humoPlugin->getConfiguration();
config["debugMode"] = true;
config["logLevel"] = "debug";
humoPlugin->setConfiguration(config);
```

**Check Logs**:

```bash
# Linux/Mac
tail -f /var/log/ekiosk/humo_payments.log

# Windows (Event Viewer)
# Look for EKiosk Humo Payments Plugin logs
```

**Command-line Debugging**:

```bash
# Run with debug output
ekiosk --debug --plugin-debug humo_payments

# Test plugin in isolation
test_plugin humo_payments --verbose

# Test card reader
pcsc_scan --verbose
```

---

## Migration notes

### Version History

| Version | Release Date | Notes                                     |
| ------- | ------------ | ----------------------------------------- |
| 1.0     | Current      | Initial stable release with full features |
| 0.9     | -            | Beta release with multistage payments     |
| 0.5-0.8 | -            | Development versions with basic features  |
| 0.1-0.4 | -            | Early prototypes                          |

### API Compatibility

- **Backward Compatibility**: ✅ Maintained
- **Forward Compatibility**: ✅ Maintained within major version
- **Breaking Changes**: Documented with migration guides

### Migration from Previous Versions

**From Legacy Payment Systems**:

- Updated to standard EKiosk plugin architecture
- Implemented IPluginFactory/IPlugin pattern
- Added proper Qt5/Qt6 compatibility
- Integrated with EKiosk services
- Updated coding standards
- Centralized metadata in HumoPaymentsFactory.cpp

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
- [Payment Service Documentation](../../docs/services/payment.md)
- [Card Reader Service Documentation](../../docs/services/cardreader.md)
- [EKiosk Developer Guide](../../docs/getting-started.md)
- [Testing Guide](../../docs/testing.md)

### Related Plugins

- **Ad Plugin**: Advertisement and sponsored payment processing
- **Payment Plugins**: Various payment processing plugins
- **ServiceMenu**: Service and maintenance interface

### Source Code Reference

- **Main Plugin**: `src/plugins/Payments/Humo/`
- **Tests**: `tests/plugins/Payments/Humo/`
- **Documentation**: `docs/plugins/humo_payments.md`

---

## Configuration Reference

### Complete Configuration Options

```ini
[HumoPaymentsPlugin]
; Dealer and transaction settings
dealerId=DEALER123
timeout=30000
retryAttempts=3
maxTransactionAmount=1000000
currency=UZS

; Card reader settings
cardReaderDevice=/dev/pcsc
pinTimeout=30
pinRetries=3

; Network and security settings
serverUrl=https://api.humo.tj/v1
connectionTimeout=10
enableSSL=true

; Offline and local data settings
offlineMode=false
localDataPath=/var/ekiosk/humo/data
maxOfflineTransactions=100

; Logging and debugging
enableLogging=true
logLevel=info
logFile=/var/log/ekiosk/humo_payments.log
debugMode=false
```

### Configuration Management

```cpp
// Load configuration from file
humoPlugin->loadConfiguration();

// Save configuration to file
humoPlugin->saveConfiguration();

// Reset to default configuration
humoPlugin->resetConfiguration();

// Validate configuration
bool isValid = humoPlugin->validateConfiguration();
```

---

## Best Practices

### Plugin Usage

- **Initialization**: Always check `isReady()` before using plugin
- **Error Handling**: Handle plugin errors gracefully
- **Configuration**: Save configuration after changes
- **Card Reader**: Verify card reader connectivity before transactions
- **PIN Security**: Never log or store PINs in plain text
- **Cleanup**: Properly release payments and resources

### Development

- **Testing**: Maintain 100% test coverage
- **Documentation**: Keep README and docs updated
- **Dependencies**: Document all dependencies
- **Compatibility**: Test on all supported platforms
- **Security**: Follow payment industry security standards

### Deployment

- **Configuration**: Provide default configuration
- **Dependencies**: Include all required dependencies including PC/SC
- **Testing**: Test on target platforms with real hardware
- **Documentation**: Include user documentation
- **Security**: Implement proper access controls

---

## Support

For issues and support:

- **GitHub Issues**: Report bugs and feature requests
- **Documentation**: Check for updates and examples
- **Community**: Join the developer community
- **Professional Support**: Contact EKiosk support team

---

## License

The Humo Payments plugin is licensed under the same terms as the main EKiosk project. See the main project license for details.
