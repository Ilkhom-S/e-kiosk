# HID Service

The HID Service manages Human Interface Device operations for the EKiosk system.

## Overview

The HID Service (`IHIDService`) handles:

- Keyboard and mouse input management
- Touchscreen input processing
- Barcode scanner integration
- RFID/NFC reader support
- Input device configuration and calibration

## Interface

```cpp
class IHIDService : public QObject {
    Q_OBJECT

public:
    enum DeviceType { Keyboard, Mouse, Touchscreen, BarcodeScanner, RFIDReader };

    /// Get connected HID devices
    virtual QStringList getConnectedDevices(DeviceType type = DeviceType::Keyboard) const = 0;

    /// Enable/disable device input
    virtual bool setDeviceEnabled(DeviceType type, bool enabled) = 0;

    /// Calibrate touchscreen
    virtual bool calibrateTouchscreen() = 0;

    /// Get touchscreen calibration status
    virtual bool isTouchscreenCalibrated() const = 0;

    /// Read barcode data
    virtual QString readBarcode(int timeoutMs = 5000) = 0;

    /// Read RFID/NFC tag
    virtual QString readRFIDTag(int timeoutMs = 5000) = 0;

    /// Send keyboard input
    virtual bool sendKeyboardInput(const QString &text) = 0;

    /// Get device status
    virtual QVariantMap getDeviceStatus(DeviceType type) const = 0;

    // ... additional methods for HID management
};
```

## Device Management

### Connected Devices

```cpp
// Get HID service from core
auto hidService = core->getHIDService();

if (!hidService) {
    LOG(log, LogLevel::Error, "HID service not available");
    return;
}

// Get connected keyboards
QStringList keyboards = hidService->getConnectedDevices(IHIDService::Keyboard);
LOG(log, LogLevel::Info, QString("Connected keyboards: %1").arg(keyboards.join(", ")));

// Get connected barcode scanners
QStringList scanners = hidService->getConnectedDevices(IHIDService::BarcodeScanner);
LOG(log, LogLevel::Info, QString("Connected scanners: %1").arg(scanners.join(", ")));
```

### Device Control

```cpp
// Disable keyboard input during payment processing
bool disabled = hidService->setDeviceEnabled(IHIDService::Keyboard, false);

if (disabled) {
    LOG(log, LogLevel::Info, "Keyboard input disabled");
} else {
    LOG(log, LogLevel::Error, "Failed to disable keyboard");
}

// Re-enable keyboard after payment
hidService->setDeviceEnabled(IHIDService::Keyboard, true);
```

## Touchscreen Operations

### Touchscreen Calibration

```cpp
// Check if touchscreen needs calibration
if (!hidService->isTouchscreenCalibrated()) {
    LOG(log, LogLevel::Info, "Touchscreen calibration required");

    // Start calibration process
    bool calibrated = hidService->calibrateTouchscreen();

    if (calibrated) {
        LOG(log, LogLevel::Info, "Touchscreen calibrated successfully");
    } else {
        LOG(log, LogLevel::Error, "Touchscreen calibration failed");
    }
}
```

### Touchscreen Status

```cpp
// Get touchscreen status
QVariantMap touchscreenStatus = hidService->getDeviceStatus(IHIDService::Touchscreen);

bool connected = touchscreenStatus["connected"].toBool();
bool calibrated = touchscreenStatus["calibrated"].toBool();
QString resolution = touchscreenStatus["resolution"].toString();

LOG(log, LogLevel::Info,
    QString("Touchscreen - Connected: %1, Calibrated: %2, Resolution: %3")
    .arg(connected).arg(calibrated).arg(resolution));
```

## Barcode Scanner Integration

### Reading Barcodes

```cpp
// Read barcode with timeout
QString barcode = hidService->readBarcode(10000);  // 10 second timeout

if (!barcode.isEmpty()) {
    LOG(log, LogLevel::Info, QString("Barcode scanned: %1").arg(barcode));

    // Process barcode (lookup product, etc.)
    processBarcode(barcode);
} else {
    LOG(log, LogLevel::Warning, "Barcode read timeout or failed");
}
```

### Asynchronous Barcode Reading

```cpp
void startBarcodeScanning() {
    // Start background barcode reading
    QTimer::singleShot(100, this, &BarcodeHandler::scanBarcodeAsync);
}

void scanBarcodeAsync() {
    QString barcode = hidService->readBarcode(5000);

    if (!barcode.isEmpty()) {
        // Process barcode
        processScannedBarcode(barcode);
    } else {
        // Continue scanning
        QTimer::singleShot(1000, this, &BarcodeHandler::scanBarcodeAsync);
    }
}
```

## RFID/NFC Operations

### Reading RFID Tags

```cpp
// Read RFID/NFC tag
QString tagId = hidService->readRFIDTag(3000);  // 3 second timeout

if (!tagId.isEmpty()) {
    LOG(log, LogLevel::Info, QString("RFID tag detected: %1").arg(tagId));

    // Process tag (user authentication, product lookup, etc.)
    processRFIDTag(tagId);
} else {
    LOG(log, LogLevel::Warning, "RFID tag read failed");
}
```

### RFID Authentication

```cpp
bool authenticateUserWithRFID() {
    LOG(log, LogLevel::Info, "Please present RFID card for authentication");

    QString cardId = hidService->readRFIDTag(15000);  // 15 second timeout

    if (cardId.isEmpty()) {
        LOG(log, LogLevel::Warning, "RFID authentication timeout");
        return false;
    }

    // Validate card against database
    bool valid = validateRFIDCard(cardId);

    if (valid) {
        LOG(log, LogLevel::Info, QString("RFID authentication successful: %1").arg(cardId));
        return true;
    } else {
        LOG(log, LogLevel::Error, QString("Invalid RFID card: %1").arg(cardId));
        return false;
    }
}
```

## Keyboard Operations

### Sending Keyboard Input

```cpp
// Send text input to focused field
bool sent = hidService->sendKeyboardInput("Hello World");

if (sent) {
    LOG(log, LogLevel::Info, "Keyboard input sent successfully");
} else {
    LOG(log, LogLevel::Error, "Failed to send keyboard input");
}
```

### Keyboard Status

```cpp
// Get keyboard device status
QVariantMap keyboardStatus = hidService->getDeviceStatus(IHIDService::Keyboard);

bool enabled = keyboardStatus["enabled"].toBool();
bool connected = keyboardStatus["connected"].toBool();
QString layout = keyboardStatus["layout"].toString();

LOG(log, LogLevel::Info,
    QString("Keyboard - Enabled: %1, Connected: %2, Layout: %3")
    .arg(enabled).arg(connected).arg(layout));
```

## Usage in Plugins

HID Service is commonly used in user interface plugins:

```cpp
class UserInterfacePlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mHIDService = mCore->getHIDService();
            mEventService = mCore->getEventService();
            mLog = kernel->getLog("UserInterface");
        }

        return true;
    }

    void startProductScanning() {
        if (!mHIDService) {
            LOG(mLog, LogLevel::Error, "HID service not available");
            return;
        }

        // Disable keyboard during scanning
        mHIDService->setDeviceEnabled(IHIDService::Keyboard, false);

        // Start barcode scanning
        scanNextProduct();
    }

    void scanNextProduct() {
        LOG(mLog, LogLevel::Info, "Ready to scan product barcode");

        QString barcode = mHIDService->readBarcode(10000);

        if (!barcode.isEmpty()) {
            // Process scanned product
            bool processed = processScannedProduct(barcode);

            if (processed) {
                LOG(mLog, LogLevel::Info, QString("Product scanned: %1").arg(barcode));

                // Publish product scanned event
                QVariantMap eventData;
                eventData["barcode"] = barcode;
                eventData["timestamp"] = QDateTime::currentDateTime();
                mEventService->publish("product.scanned", eventData);

                // Continue scanning
                scanNextProduct();
            } else {
                LOG(mLog, LogLevel::Error, QString("Invalid product barcode: %1").arg(barcode));
                showError("Invalid product");
            }
        } else {
            LOG(mLog, LogLevel::Warning, "Barcode scan timeout");
            showError("Scan timeout - please try again");
        }
    }

    void stopProductScanning() {
        // Re-enable keyboard
        mHIDService->setDeviceEnabled(IHIDService::Keyboard, true);

        LOG(mLog, LogLevel::Info, "Product scanning stopped");
    }

    bool authenticateUser() {
        LOG(mLog, LogLevel::Info, "Starting RFID authentication");

        QString cardId = mHIDService->readRFIDTag(10000);

        if (cardId.isEmpty()) {
            LOG(mLog, LogLevel::Warning, "RFID authentication timeout");
            return false;
        }

        // Validate card
        bool authenticated = validateUserCard(cardId);

        if (authenticated) {
            LOG(mLog, LogLevel::Info, QString("User authenticated: %1").arg(cardId));

            // Publish authentication event
            QVariantMap eventData;
            eventData["userId"] = cardId;
            eventData["method"] = "rfid";
            mEventService->publish("user.authenticated", eventData);

            return true;
        } else {
            LOG(mLog, LogLevel::Error, QString("Invalid RFID card: %1").arg(cardId));
            return false;
        }
    }

private:
    bool processScannedProduct(const QString &barcode) {
        // Lookup product in database
        // Add to cart, update display, etc.
        return true; // Simplified
    }

    bool validateUserCard(const QString &cardId) {
        // Check card validity in database
        return true; // Simplified
    }

    void showError(const QString &message) {
        // Display error on UI
        LOG(mLog, LogLevel::Error, QString("UI Error: %1").arg(message));
    }

private:
    IHIDService *mHIDService;
    IEventService *mEventService;
    ILog *mLog;
};
```

## Error Handling

```cpp
try {
    // Check device availability
    QStringList scanners = hidService->getConnectedDevices(IHIDService::BarcodeScanner);

    if (scanners.isEmpty()) {
        throw std::runtime_error("No barcode scanners connected");
    }

    // Attempt barcode read
    QString barcode = hidService->readBarcode(5000);

    if (barcode.isEmpty()) {
        throw std::runtime_error("Barcode read timeout");
    }

    // Process barcode
    processBarcode(barcode);

} catch (const std::runtime_error &e) {
    LOG(log, LogLevel::Error, QString("HID service error: %1").arg(e.what()));

    // Handle error - show user message, fallback input method, etc.
    handleHIDError(e.what());

} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Unexpected HID error: %1").arg(e.what()));
}
```

## Device Configuration

```cpp
// Configure HID devices through settings
void configureHIDDevices() {
    auto settings = core->getSettingsService();

    // Configure barcode scanner timeout
    int scannerTimeout = settings->getValue("HID/ScannerTimeout", 5000).toInt();

    // Configure RFID reader settings
    bool rfidEnabled = settings->getValue("HID/RFIDEnabled", true).toBool();

    // Configure touchscreen calibration
    bool autoCalibrate = settings->getValue("HID/AutoCalibrateTouchscreen", false).toBool();

    if (autoCalibrate && !hidService->isTouchscreenCalibrated()) {
        hidService->calibrateTouchscreen();
    }
}
```

## Security Considerations

- Validate input data to prevent injection attacks
- Implement proper authentication for RFID/NFC operations
- Log security-relevant HID operations
- Consider input validation for barcode data

## Dependencies

- Device Service (for HID hardware management)
- Settings Service (for device configuration)
- Event Service (for input event notifications)
- Database Service (for user/card validation)

## See Also

- [Device Service](device.md) - HID hardware management
- [Settings Service](settings.md) - HID device configuration
- [Event Service](event.md) - Input event notifications
- [Database Service](database.md) - User/card validation storage
