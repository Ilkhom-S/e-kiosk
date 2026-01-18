# Ad Plugin

The Ad Plugin provides advertising content management and payment processing functionality for the EKiosk system. It implements a payment factory that handles advertisement-based payments and integrates with the broader advertising ecosystem.

## Overview

The Ad Plugin manages:

- **Payment Processing**: Advertisement-based payment transactions
- **Content Integration**: Links payments to advertisement display
- **Campaign Management**: Integration with advertising campaigns
- **Remote Advertisement Updates**: Content management through remote services

## Main Entry Point

**Primary Plugin**: `PaymentFactory.cpp` - Implements payment factory for advertisement transactions
**Supporting Components**: Ad service integration and content management

## Features

- **Payment Factory**: Processes advertisement-sponsored payments
- **Ad Integration**: Links payment transactions to advertisement display
- **Content Management**: Manages advertisement content and campaigns
- **Remote Updates**: Supports remote advertisement content updates
- **Statistics**: Tracks payment and advertisement performance

## Dependencies

- **Services**:
  - Database Service (campaign storage)
  - Network Service (remote updates)
  - Payment Service (sponsored content)
  - Event Service (campaign notifications)

- **Modules**:
  - SysUtils (system utilities)
  - PaymentBase (payment processing)
  - NetworkTaskManager (HTTP communications)
  - AdBackend (advertisement backend)

## Configuration

The plugin supports the following configuration options:

```json
{
  "ad": {
    "displayInterval": 300,
    "maxCampaigns": 10,
    "contentPath": "/opt/ekiosk/ads",
    "enableRemoteUpdates": true,
    "statisticsEnabled": true
  }
}
```

## Usage

### Basic Usage

```cpp
// Plugin is automatically loaded by the Plugin Service
// Access through service interfaces

auto adService = core->getAdService();
if (adService) {
    // Schedule advertisement display
    adService->scheduleCampaign(campaignData);

    // Get current advertisement
    auto currentAd = adService->getCurrentAdvertisement();
}
```

### Integration with Payment Service

```cpp
// Handle sponsored advertisement payments
void handleSponsoredAd(const QVariantMap &paymentData) {
    // Process payment for sponsored content
    bool success = paymentService->processPayment(paymentData);

    if (success) {
        // Display sponsored advertisement
        adService->displaySponsoredAd(paymentData.value("adId").toString());
    }
}
```

## Plugin Structure

```text
src/plugins/Ad/
├── CMakeLists.txt           # Build configuration
├── README.md               # Plugin documentation
└── src/                    # Source files
    ├── PaymentFactory.cpp         # Main plugin entry point - payment factory
    ├── AdPluginFactory.cpp        # Plugin metadata and factory base class
    ├── AdService.h/.cpp           # Ad service implementation
    ├── AdPayment.h/.cpp           # Payment processing for advertisements
    ├── AdStatistics.h/.cpp        # Statistics and reporting
    ├── AdRemoteUpdate.h/.cpp      # Remote content update functionality
    ├── AdPluginImpl.h/.cpp        # Plugin implementation
    ├── AdRemotePlugin.h/.cpp      # Remote management
    ├── AdSourcePlugin.h/.cpp      # Content source
    ├── PaymentFactoryBase.h/.cpp  # Base payment factory
    └── PluginLibraryDefinition.h/.cpp # Legacy definitions
```

**Main Entry Point**: `PaymentFactory.cpp` contains the `REGISTER_PLUGIN` macro and `CreatePaymentFactory` function that registers the plugin with the EKiosk system.

**Metadata**: `AdPluginFactory.cpp` defines the static metadata (name, description, version, author) used by the plugin system.

## Building

The plugin is built using the standard EKiosk CMake build system:

```bash
cmake --build build/msvc --target ad
```

## Testing

Run plugin tests:

```bash
ctest -R ad_plugin_test
```

Or run all plugin tests:

```bash
ctest -R ad
```

## API Reference

### AdService Interface

```cpp
class AdService : public QObject, public SDK::PaymentProcessor::IService {
public:
    // Campaign management
    bool scheduleCampaign(const QVariantMap &campaign);
    bool cancelCampaign(const QString &campaignId);
    QList<QVariantMap> getActiveCampaigns();

    // Content management
    bool loadAdvertisement(const QString &adId);
    QVariantMap getCurrentAdvertisement();
    bool displaySponsoredAd(const QString &adId);

    // Statistics
    QVariantMap getStatistics(const QString &campaignId);
    bool resetStatistics(const QString &campaignId);
};
```

## Troubleshooting

### Common Issues

- **Campaign not displaying**: Check campaign schedule and content availability
- **Payment integration failed**: Verify Payment Service configuration
- **Remote updates not working**: Check Network Service connectivity
- **Statistics not updating**: Ensure proper database permissions

### Debug Logging

Enable debug logging for the Ad Plugin:

```cpp
ILog *log = environment->getLog("AdPlugin");
LOG(log, LogLevel::Debug, "Debug message");
```

## Migration Notes

This plugin has been migrated from the original TerminalClient implementation to follow the new EKiosk plugin architecture:

- **Qt5/Qt6 Compatibility**: Updated to support both Qt versions
- **Service Integration**: Now properly integrates with EKiosk services
- **Plugin Architecture**: Follows standard IPluginFactory/IPlugin pattern
- **Code Standards**: Updated to follow EKiosk coding standards with Russian comments
- **Registration Refactor**: Main entry point moved to `PaymentFactory.cpp` with `REGISTER_PLUGIN` macro, metadata centralized in `AdPluginFactory.cpp`

## See Also

- [Plugin System Architecture](../../docs/plugins/README.md)
- [Ad Service Documentation](../../docs/services/ad.md)
- [Payment Service Documentation](../../docs/services/payment.md)
