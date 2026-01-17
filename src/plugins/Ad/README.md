# Ad Plugin

The Ad Plugin provides advertising content management and scheduling functionality for the EKiosk system.

## Overview

The Ad Plugin manages:

- Advertising campaign scheduling and display
- Content management for advertisements
- Payment integration for sponsored content
- Remote advertisement updates
- Statistics and reporting

## Features

- **Campaign Management**: Schedule and manage advertising campaigns
- **Content Types**: Support for images, videos, and interactive content
- **Payment Integration**: Monetization through sponsored advertisements
- **Remote Management**: Update advertisements remotely
- **Statistics**: Track advertisement performance and engagement

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
    ├── AdPluginFactory.h/.cpp    # Plugin factory
    ├── AdPluginImpl.h/.cpp       # Plugin implementation
    ├── AdService.h/.cpp          # Ad service
    ├── AdPayment.h/.cpp          # Payment integration
    ├── AdRemotePlugin.h/.cpp     # Remote management
    ├── AdSourcePlugin.h/.cpp     # Content source
    ├── PaymentFactory.h/.cpp     # Payment factory
    ├── PaymentFactoryBase.h/.cpp # Base payment factory
    ├── PluginLibraryDefinition.h/.cpp # Legacy definitions
    └── plugin.json               # Qt plugin metadata
```

## Building

The plugin is built using the standard EKiosk CMake build system:

```bash
cmake --build build/msvc --target ad
```

## Testing

Run plugin tests:

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

## See Also

- [Plugin System Architecture](../../docs/plugins/README.md)
- [Ad Service Documentation](../../docs/services/ad.md)
- [Payment Service Documentation](../../docs/services/payment.md)
