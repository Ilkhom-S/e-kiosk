# Ad Service

The Ad Service manages advertisement display and scheduling for the EKiosk system.

## Overview

The Ad Service (`IAdService`) handles:

- Advertisement content management
- Ad scheduling and rotation
- Ad display coordination
- Ad performance tracking
- Ad content validation
- Integration with display backends

## Interface

```cpp
class IAdService : public QObject {
    Q_OBJECT

public:
    enum AdStatus { Active, Inactive, Expired, Error };
    enum AdType { Image, Video, Text, Html };

    struct AdContent {
        QString id;
        QString name;
        AdType type;
        QString contentPath;  // File path or URL
        QString targetUrl;    // Click-through URL
        QDateTime startDate;
        QDateTime endDate;
        int duration;         // Display duration in seconds
        int priority;         // Display priority (higher = more frequent)
        QVariantMap metadata; // Additional ad data
    };

    /// Load advertisement content
    virtual bool loadAdContent(const AdContent &content) = 0;

    /// Remove advertisement content
    virtual bool removeAdContent(const QString &adId) = 0;

    /// Get advertisement by ID
    virtual AdContent getAdContent(const QString &adId) const = 0;

    /// Get all active advertisements
    virtual QList<AdContent> getActiveAds() const = 0;

    /// Schedule advertisement display
    virtual bool scheduleAd(const QString &adId, const QDateTime &startTime,
                           const QDateTime &endTime) = 0;

    /// Display advertisement immediately
    virtual bool displayAd(const QString &adId) = 0;

    /// Stop current advertisement
    virtual bool stopAd() = 0;

    /// Get current ad display status
    virtual AdStatus getAdStatus() const = 0;

    /// Set advertisement rotation schedule
    virtual bool setAdRotation(const QStringList &adIds, int intervalSeconds) = 0;

    /// Get advertisement statistics
    virtual QVariantMap getAdStatistics(const QString &adId) const = 0;

    // ... additional methods for ad management
};
```

## Ad Content Management

### Loading Ad Content

```cpp
// Get ad service from core
auto adService = core->getAdService();

if (!adService) {
    LOG(log, LogLevel::Error, "Ad service not available");
    return;
}

// Create ad content
IAdService::AdContent ad;
ad.id = "promo_001";
ad.name = "Summer Sale Promotion";
ad.type = IAdService::Image;
ad.contentPath = "ads/summer_sale.jpg";
ad.targetUrl = "https://example.com/summer-sale";
ad.startDate = QDateTime::currentDateTime();
ad.endDate = QDateTime::currentDateTime().addDays(30);
ad.duration = 15;  // 15 seconds
ad.priority = 5;   // Medium priority

// Load the advertisement
bool loaded = adService->loadAdContent(ad);

if (loaded) {
    LOG(log, LogLevel::Info, QString("Ad loaded: %1").arg(ad.name));
} else {
    LOG(log, LogLevel::Error, QString("Failed to load ad: %1").arg(ad.name));
}
```

### Managing Multiple Ads

```cpp
// Load multiple advertisements
QList<IAdService::AdContent> ads = loadAdsFromDatabase();

foreach (const auto &ad, ads) {
    bool loaded = adService->loadAdContent(ad);

    if (loaded) {
        LOG(log, LogLevel::Info, QString("Loaded ad: %1").arg(ad.name));
    } else {
        LOG(log, LogLevel::Warning, QString("Failed to load ad: %1").arg(ad.name));
    }
}
```

### Removing Ads

```cpp
// Remove expired advertisement
bool removed = adService->removeAdContent("expired_ad_001");

if (removed) {
    LOG(log, LogLevel::Info, "Expired ad removed");
} else {
    LOG(log, LogLevel::Error, "Failed to remove expired ad");
}
```

## Ad Scheduling

### Scheduling Ads

```cpp
// Schedule ad for specific time period
QDateTime startTime = QDateTime::currentDateTime().addDays(1);  // Tomorrow
QDateTime endTime = startTime.addDays(7);  // One week

bool scheduled = adService->scheduleAd("promo_001", startTime, endTime);

if (scheduled) {
    LOG(log, LogLevel::Info, "Ad scheduled successfully");
} else {
    LOG(log, LogLevel::Error, "Failed to schedule ad");
}
```

### Ad Rotation

```cpp
// Set up ad rotation with multiple ads
QStringList adIds = {"promo_001", "seasonal_002", "product_003"};
int rotationInterval = 30;  // 30 seconds between ads

bool rotationSet = adService->setAdRotation(adIds, rotationInterval);

if (rotationSet) {
    LOG(log, LogLevel::Info, "Ad rotation configured");
} else {
    LOG(log, LogLevel::Error, "Failed to configure ad rotation");
}
```

## Ad Display Control

### Immediate Display

```cpp
// Display advertisement immediately
bool displayed = adService->displayAd("urgent_announcement");

if (displayed) {
    LOG(log, LogLevel::Info, "Ad displayed immediately");
} else {
    LOG(log, LogLevel::Error, "Failed to display ad");
}
```

### Display Control

```cpp
// Control ad display
void manageAdDisplay() {
    // Check current status
    IAdService::AdStatus status = adService->getAdStatus();

    switch (status) {
        case IAdService::Active:
            LOG(log, LogLevel::Info, "Ad is currently displaying");
            break;
        case IAdService::Inactive:
            LOG(log, LogLevel::Info, "No ad is displaying");
            break;
        case IAdService::Expired:
            LOG(log, LogLevel::Warning, "Current ad has expired");
            break;
        case IAdService::Error:
            LOG(log, LogLevel::Error, "Ad display error");
            break;
    }

    // Stop current ad if needed
    if (status == IAdService::Active) {
        adService->stopAd();
        LOG(log, LogLevel::Info, "Ad display stopped");
    }
}
```

## Ad Statistics and Analytics

### Tracking Performance

```cpp
// Get advertisement statistics
QVariantMap stats = adService->getAdStatistics("promo_001");

int displayCount = stats.value("displayCount", 0).toInt();
int clickCount = stats.value("clickCount", 0).toInt();
double clickRate = stats.value("clickRate", 0.0).toDouble();

LOG(log, LogLevel::Info, QString("Ad stats - Displays: %1, Clicks: %2, Rate: %3%")
    .arg(displayCount).arg(clickCount).arg(clickRate * 100));
```

### Analytics Integration

```cpp
void trackAdPerformance() {
    // Get all active ads
    QList<IAdService::AdContent> activeAds = adService->getActiveAds();

    foreach (const auto &ad, activeAds) {
        QVariantMap stats = adService->getAdStatistics(ad.id);

        // Send to analytics service
        sendToAnalytics("ad_performance", {
            {"ad_id", ad.id},
            {"ad_name", ad.name},
            {"displays", stats.value("displayCount", 0)},
            {"clicks", stats.value("clickCount", 0)},
            {"impressions", stats.value("impressionCount", 0)}
        });
    }
}
```

## Usage in Plugins

Ad Service is commonly used in display and content management plugins:

```cpp
class AdDisplayPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mAdService = mCore->getAdService();
            mEventService = mCore->getEventService();
            mLog = kernel->getLog("AdDisplay");
        }

        return true;
    }

    void loadCampaignAds() {
        // Load ads from campaign database
        QList<IAdService::AdContent> campaignAds = loadCampaignFromDatabase();

        foreach (const auto &ad, campaignAds) {
            bool loaded = mAdService->loadAdContent(ad);

            if (loaded) {
                LOG(mLog, LogLevel::Info, QString("Campaign ad loaded: %1").arg(ad.name));

                // Schedule based on campaign settings
                scheduleCampaignAd(ad);
            } else {
                LOG(mLog, LogLevel::Error, QString("Failed to load campaign ad: %1").arg(ad.name));
            }
        }
    }

    void scheduleCampaignAd(const IAdService::AdContent &ad) {
        // Schedule ad based on priority and time slots
        QDateTime startTime = ad.startDate;
        QDateTime endTime = ad.endDate;

        bool scheduled = mAdService->scheduleAd(ad.id, startTime, endTime);

        if (scheduled) {
            LOG(mLog, LogLevel::Info, QString("Ad scheduled: %1 from %2 to %3")
                .arg(ad.name)
                .arg(startTime.toString())
                .arg(endTime.toString()));
        }
    }

    void handleIdleTimeAds() {
        // When kiosk is idle, show advertisements
        if (isKioskIdle()) {
            // Get available ads for idle time
            QStringList idleAds = getIdleTimeAds();

            if (!idleAds.isEmpty()) {
                // Set up rotation for idle time
                mAdService->setAdRotation(idleAds, 45);  // 45 second intervals

                LOG(mLog, LogLevel::Info, "Idle time ad rotation started");
            }
        } else {
            // Stop ad rotation when user is active
            mAdService->stopAd();
            LOG(mLog, LogLevel::Info, "Idle time ad rotation stopped");
        }
    }

    void displayPromotionalContent() {
        // Display special promotions
        QString promoAdId = getCurrentPromotionAd();

        if (!promoAdId.isEmpty()) {
            bool displayed = mAdService->displayAd(promoAdId);

            if (displayed) {
                LOG(mLog, LogLevel::Info, QString("Promotion displayed: %1").arg(promoAdId));

                // Track promotion display
                trackPromotionDisplay(promoAdId);
            } else {
                LOG(mLog, LogLevel::Error, QString("Failed to display promotion: %1").arg(promoAdId));
            }
        }
    }

    void manageAdContent() {
        // Clean up expired ads
        QList<IAdService::AdContent> activeAds = mAdService->getActiveAds();

        foreach (const auto &ad, activeAds) {
            if (ad.endDate < QDateTime::currentDateTime()) {
                // Ad has expired
                bool removed = mAdService->removeAdContent(ad.id);

                if (removed) {
                    LOG(mLog, LogLevel::Info, QString("Expired ad removed: %1").arg(ad.name));
                } else {
                    LOG(mLog, LogLevel::Warning, QString("Failed to remove expired ad: %1").arg(ad.name));
                }
            }
        }

        // Load new ads from content management system
        loadNewAdsFromCMS();
    }

    void handleAdEvents() {
        // Subscribe to ad-related events
        mEventService->subscribe("ad.displayed", [this](const QVariantMap &eventData) {
            QString adId = eventData.value("adId").toString();
            LOG(mLog, LogLevel::Info, QString("Ad displayed: %1").arg(adId));

            // Update display statistics
            updateAdStatistics(adId, "displayed");
        });

        mEventService->subscribe("ad.clicked", [this](const QVariantMap &eventData) {
            QString adId = eventData.value("adId").toString();
            QString targetUrl = eventData.value("targetUrl").toString();

            LOG(mLog, LogLevel::Info, QString("Ad clicked: %1 -> %2").arg(adId, targetUrl));

            // Handle click-through
            handleAdClick(adId, targetUrl);

            // Update click statistics
            updateAdStatistics(adId, "clicked");
        });
    }

    void updateAdStatistics(const QString &adId, const QString &eventType) {
        // Get current statistics
        QVariantMap stats = mAdService->getAdStatistics(adId);

        // Update counters
        if (eventType == "displayed") {
            int displays = stats.value("displayCount", 0).toInt() + 1;
            stats["displayCount"] = displays;
        } else if (eventType == "clicked") {
            int clicks = stats.value("clickCount", 0).toInt() + 1;
            stats["clickCount"] = clicks;
        }

        // Calculate click rate
        int displays = stats.value("displayCount", 0).toInt();
        int clicks = stats.value("clickCount", 0).toInt();

        if (displays > 0) {
            double clickRate = static_cast<double>(clicks) / displays;
            stats["clickRate"] = clickRate;
        }

        // Store updated statistics
        saveAdStatistics(adId, stats);
    }

    void handleAdClick(const QString &adId, const QString &targetUrl) {
        // Validate URL
        if (!isValidUrl(targetUrl)) {
            LOG(mLog, LogLevel::Warning, QString("Invalid ad click URL: %1").arg(targetUrl));
            return;
        }

        // Open URL in appropriate way (browser, embedded web view, etc.)
        openAdUrl(targetUrl);

        LOG(mLog, LogLevel::Info, QString("Ad click handled: %1 -> %2").arg(adId, targetUrl));
    }

private:
    IAdService *mAdService;
    IEventService *mEventService;
    ILog *mLog;
};
```

## Error Handling

```cpp
try {
    // Validate ad content before loading
    if (ad.contentPath.isEmpty()) {
        throw std::invalid_argument("Ad content path cannot be empty");
    }

    if (!QFile::exists(ad.contentPath)) {
        throw std::runtime_error("Ad content file does not exist");
    }

    // Check ad service availability
    if (!adService) {
        throw std::runtime_error("Ad service not available");
    }

    // Load ad content
    if (!adService->loadAdContent(ad)) {
        throw std::runtime_error("Failed to load ad content");
    }

} catch (const std::invalid_argument &e) {
    LOG(log, LogLevel::Error, QString("Invalid ad data: %1").arg(e.what()));

} catch (const std::runtime_error &e) {
    LOG(log, LogLevel::Error, QString("Ad service error: %1").arg(e.what()));

    // Handle error - show fallback content, retry, etc.
    showFallbackAd();

} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Unexpected ad error: %1").arg(e.what()));
}
```

## Ad Content Validation

```cpp
bool validateAdContent(const IAdService::AdContent &ad) {
    // Validate required fields
    if (ad.id.isEmpty() || ad.name.isEmpty()) {
        LOG(log, LogLevel::Error, "Ad ID and name are required");
        return false;
    }

    // Validate content path
    if (ad.contentPath.isEmpty()) {
        LOG(log, LogLevel::Error, "Ad content path is required");
        return false;
    }

    // Check file existence for local files
    if (!ad.contentPath.startsWith("http") && !QFile::exists(ad.contentPath)) {
        LOG(log, LogLevel::Error, QString("Ad content file not found: %1").arg(ad.contentPath));
        return false;
    }

    // Validate dates
    if (ad.startDate > ad.endDate) {
        LOG(log, LogLevel::Error, "Ad start date cannot be after end date");
        return false;
    }

    // Validate duration
    if (ad.duration <= 0) {
        LOG(log, LogLevel::Error, "Ad duration must be positive");
        return false;
    }

    // Validate priority
    if (ad.priority < 0 || ad.priority > 10) {
        LOG(log, LogLevel::Error, "Ad priority must be between 0 and 10");
        return false;
    }

    return true;
}
```

## Performance Considerations

- Pre-load ad content during idle times
- Use compressed image formats for faster loading
- Implement ad content caching
- Monitor memory usage for video ads
- Use background loading for large ad files

## Security Considerations

- Validate ad content URLs to prevent malicious redirects
- Sanitize ad metadata to prevent injection attacks
- Implement content filtering for inappropriate ads
- Secure ad statistics storage and access
- Validate file types and content before loading

## Integration with Display Backends

```cpp
// Coordinate with display backend for ad rendering
void displayAdOnBackend(const QString &adId) {
    auto displayBackend = core->getDisplayBackend();

    if (!displayBackend) {
        LOG(log, LogLevel::Error, "Display backend not available");
        return;
    }

    // Get ad content
    IAdService::AdContent ad = adService->getAdContent(adId);

    if (ad.id.isEmpty()) {
        LOG(log, LogLevel::Error, QString("Ad not found: %1").arg(adId));
        return;
    }

    // Display based on ad type
    switch (ad.type) {
        case IAdService::Image:
            displayBackend->showImage(ad.contentPath, ad.duration * 1000);
            break;
        case IAdService::Video:
            displayBackend->playVideo(ad.contentPath);
            break;
        case IAdService::Text:
            displayBackend->showText(ad.contentPath, ad.duration * 1000);
            break;
        case IAdService::Html:
            displayBackend->showHtml(ad.contentPath, ad.duration * 1000);
            break;
    }

    LOG(log, LogLevel::Info, QString("Ad displayed on backend: %1").arg(ad.name));
}
```

## Dependencies

- Settings Service (for ad configuration)
- Event Service (for ad event notifications)
- Database Service (for ad content storage)
- Display Backend (for ad rendering)

## See Also

- [Settings Service](settings.md) - Ad configuration
- [Event Service](event.md) - Ad event notifications
- [Database Service](database.md) - Ad content storage
- [Display Backend](../../plugins/GraphicBackends/) - Ad rendering
