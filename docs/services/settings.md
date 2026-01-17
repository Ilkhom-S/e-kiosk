# Settings Service

The Settings Service provides configuration management and application settings access for EKiosk.

## Overview

The Settings Service (`ISettingsService`) manages:

- Application configuration storage and retrieval
- Settings adapters for different configuration sources
- Runtime configuration changes
- Terminal and system settings

## Interface

```cpp
class ISettingsService {
public:
    /// Get settings adapter by name
    virtual ISettingsAdapter *getAdapter(const QString &aAdapterName) = 0;

    /// Save complete configuration
    virtual bool saveConfiguration() = 0;
};
```

## Settings Adapters

### Terminal Settings

Terminal settings provide access to kiosk-specific configuration:

```cpp
auto settings = core->getSettingsService();
ISettingsAdapter *terminalAdapter = settings->getAdapter("TerminalSettings");

if (terminalAdapter) {
    SDK::PaymentProcessor::TerminalSettings *terminalSettings =
        terminalAdapter->getTerminalSettings();

    // Access terminal configuration
    QString terminalId = terminalSettings->getTerminalId();
    QString merchantId = terminalSettings->getMerchantId();
    // ... other terminal settings
}
```

### Custom Settings Adapters

Services can provide their own settings adapters:

```cpp
// Network settings
auto networkAdapter = settings->getAdapter("NetworkSettings");
// Configure network timeouts, proxies, etc.

// Crypt settings
auto cryptAdapter = settings->getAdapter("CryptSettings");
// Configure cryptographic parameters

// Payment settings
auto paymentAdapter = settings->getAdapter("PaymentSettings");
// Configure payment processor settings
```

## Configuration Management

### Saving Configuration

```cpp
// Save all current settings
bool success = settings->saveConfiguration();
if (!success) {
    LOG(log, LogLevel::Error, "Failed to save configuration");
}
```

### Runtime Configuration Changes

```cpp
// Update settings at runtime
terminalSettings->setTerminalId("NEW_TERMINAL_001");
terminalSettings->setMerchantId("NEW_MERCHANT_123");

// Save changes
settings->saveConfiguration();
```

## Usage in Plugins

Settings Service is used throughout the application for configuration access:

```cpp
class PaymentPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mSettings = mCore->getSettingsService();

            // Get plugin-specific settings
            mPluginSettings = mSettings->getAdapter("PaymentPluginSettings");

            // Get terminal settings for context
            auto terminalAdapter = mSettings->getAdapter("TerminalSettings");
            if (terminalAdapter) {
                mTerminalSettings = terminalAdapter->getTerminalSettings();
            }
        }

        return true;
    }

    void processPayment() {
        // Use settings for payment configuration
        QString processorUrl = mPluginSettings->getValue("processor.url").toString();
        int timeout = mPluginSettings->getValue("timeout", 30).toInt();

        // Use terminal settings for transaction context
        QString terminalId = mTerminalSettings->getTerminalId();

        // Process payment with configuration
        // ...
    }

private:
    ISettingsService *mSettings;
    ISettingsAdapter *mPluginSettings;
    SDK::PaymentProcessor::TerminalSettings *mTerminalSettings;
};
```

## Settings Types

### Built-in Settings

- **TerminalSettings**: Terminal identification, merchant info, location data
- **NetworkSettings**: Connection parameters, timeouts, proxy configuration
- **CryptSettings**: Cryptographic parameters, key server configuration
- **PaymentSettings**: Payment processor configuration, transaction limits
- **GUISettings**: User interface configuration, display settings
- **PrinterSettings**: Print configuration, receipt formatting

### Custom Settings

Plugins can define their own settings adapters:

```cpp
// Plugin-specific settings
class MyPluginSettings : public ISettingsAdapter {
    // Implementation for plugin-specific configuration
};
```

## Configuration Files

Settings are typically stored in configuration files managed by the application. The exact storage mechanism depends on the platform and deployment configuration.

## Error Handling

```cpp
try {
    auto adapter = settings->getAdapter("TerminalSettings");
    if (!adapter) {
        LOG(log, LogLevel::Error, "Terminal settings adapter not available");
        return false;
    }

    auto terminalSettings = adapter->getTerminalSettings();
    if (!terminalSettings) {
        LOG(log, LogLevel::Error, "Terminal settings not available");
        return false;
    }
} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Settings error: %1").arg(e.what()));
}
```

## Thread Safety

Settings access is generally thread-safe for read operations. Write operations should be coordinated to avoid conflicts.

## Dependencies

- Core Service (for service access)
- Logging Service (for diagnostics)

## See Also

- [Network Service](network.md) - Network configuration
- [Crypt Service](crypt.md) - Cryptographic settings
- [Terminal Service](terminal.md) - Terminal configuration
