# Device Service

The Device Service manages hardware devices connected to the EKiosk system.

## Overview

The Device Service (`IDeviceService`) provides:

- Device detection and enumeration
- Device acquisition and release
- Firmware updates
- Device configuration management
- Support for printers, card readers, bill acceptors, etc.

## Interface

```cpp
class IDeviceService : public QObject {
    Q_OBJECT

public:
    enum UpdateFirmwareResult { OK = 0, NoDevice, CantUpdate };

public:
    /// Detect devices (non-blocking)
    virtual void detect(const QString &aDeviceType = QString()) = 0;

    /// Stop device detection
    virtual void stopDetection() = 0;

    /// Get device configurations
    virtual QStringList getConfigurations(bool aAllowOldConfigs = true) const = 0;

    /// Save device configurations
    virtual bool saveConfigurations(const QStringList &aConfigList) = 0;

    /// Set device initialization parameters
    virtual void setInitParameters(const QString &aDeviceType,
                                 const QVariantMap &aParameters) = 0;

    /// Acquire device instance
    virtual SDK::Driver::IDevice *acquireDevice(const QString &aInstancePath) = 0;

    // ... additional methods for device management
};
```

## Device Detection

### Automatic Detection

```cpp
auto deviceService = core->getDeviceService();

// Detect all devices
deviceService->detect();

// Detect specific device type
deviceService->detect("Printer");
deviceService->detect("CardReader");
deviceService->detect("BillAcceptor");
```

### Device Configurations

```cpp
// Get available device configurations
QStringList configs = deviceService->getConfigurations();

// Save updated configurations
bool saved = deviceService->saveConfigurations(updatedConfigs);
```

## Device Acquisition

### Acquiring Devices

```cpp
// Acquire a specific device instance
SDK::Driver::IDevice *device = deviceService->acquireDevice("/devices/printer/thermal1");

if (device) {
    // Use device
    // ...

    // Release device when done
    device->release();
}
```

### Device Initialization Parameters

```cpp
// Set initialization parameters for device type
QVariantMap params;
params["baudRate"] = 9600;
params["parity"] = "none";
params["timeout"] = 5000;

deviceService->setInitParameters("CardReader", params);
```

## Common Device Types

### Printers

```cpp
// Acquire printer device
SDK::Driver::IDevice *printer = deviceService->acquireDevice("/devices/printer/receipt1");

if (printer) {
    // Cast to printer interface
    auto printerDevice = dynamic_cast<IPrinterDevice*>(printer);

    if (printerDevice) {
        // Print receipt
        printerDevice->printReceipt(receiptData);
    }

    printer->release();
}
```

### Card Readers

```cpp
// Acquire card reader
SDK::Driver::IDevice *cardReader = deviceService->acquireDevice("/devices/cardreader/contact1");

if (cardReader) {
    auto readerDevice = dynamic_cast<ICardReaderDevice*>(cardReader);

    if (readerDevice) {
        // Read card data
        CardData card = readerDevice->readCard();

        // Process card data
        processPayment(card);
    }

    cardReader->release();
}
```

### Bill Acceptors

```cpp
// Acquire bill acceptor
SDK::Driver::IDevice *billAcceptor = deviceService->acquireDevice("/devices/billacceptor/main1");

if (billAcceptor) {
    auto acceptorDevice = dynamic_cast<IBillAcceptorDevice*>(billAcceptor);

    if (acceptorDevice) {
        // Enable bill acceptance
        acceptorDevice->enable();

        // Handle bill inserted events
        connect(acceptorDevice, &IBillAcceptorDevice::billInserted,
                this, &PaymentHandler::onBillInserted);
    }

    billAcceptor->release();
}
```

## Usage in Plugins

Device Service is commonly used in hardware interface plugins:

```cpp
class PrinterPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mDeviceService = mCore->getDeviceService();

            // Detect available printers
            mDeviceService->detect("Printer");
        }

        return true;
    }

    void printReceipt(const QString &receiptData) {
        // Acquire printer device
        SDK::Driver::IDevice *printer = mDeviceService->acquireDevice(mPrinterPath);

        if (printer) {
            auto printerDevice = dynamic_cast<IPrinterDevice*>(printer);
            if (printerDevice) {
                printerDevice->printReceipt(receiptData);
            }
            printer->release();
        } else {
            LOG(mLog, LogLevel::Error, "Printer device not available");
        }
    }

private:
    IDeviceService *mDeviceService;
    QString mPrinterPath;
    ILog *mLog;
};
```

## Firmware Updates

```cpp
// Update device firmware
IDeviceService::UpdateFirmwareResult result =
    deviceService->updateFirmware(devicePath, firmwareData);

switch (result) {
    case IDeviceService::OK:
        LOG(log, LogLevel::Info, "Firmware update successful");
        break;
    case IDeviceService::NoDevice:
        LOG(log, LogLevel::Error, "Device not found for firmware update");
        break;
    case IDeviceService::CantUpdate:
        LOG(log, LogLevel::Error, "Firmware update failed");
        break;
}
```

## Error Handling

```cpp
try {
    SDK::Driver::IDevice *device = deviceService->acquireDevice(devicePath);
    if (!device) {
        LOG(log, LogLevel::Error, QString("Failed to acquire device: %1").arg(devicePath));
        return;
    }

    // Use device safely
    // ...

} catch (const SDK::Driver::DeviceException &e) {
    LOG(log, LogLevel::Error, QString("Device error: %1").arg(e.what()));
} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Unexpected device error: %1").arg(e.what()));
}
```

## Device Events

Devices often emit signals for status changes:

```cpp
// Connect to device status signals
connect(device, &SDK::Driver::IDevice::statusChanged,
        this, &DeviceHandler::onDeviceStatusChanged);

connect(device, &SDK::Driver::IDevice::errorOccurred,
        this, &DeviceHandler::onDeviceError);
```

## Configuration

Device configuration is managed through the Settings Service:

```cpp
auto settings = core->getSettingsService();
auto deviceSettings = settings->getAdapter("DeviceSettings");
// Configure device parameters, timeouts, etc.
```

## Thread Safety

Device operations should be performed on the appropriate thread. Many devices have their own event loops and should not be accessed from multiple threads simultaneously.

## Dependencies

- Settings Service (for device configuration)
- Event Service (for device status notifications)
- Logging Service (for diagnostics)

## See Also

- [Printer Service](printer.md) - Print management
- [Settings Service](settings.md) - Device configuration
- [Event Service](event.md) - Device status notifications
