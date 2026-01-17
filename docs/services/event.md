# Event Service

The Event Service provides event distribution and messaging for the EKiosk system.

## Overview

The Event Service (`IEventService`) manages:

- Event publishing and subscription
- Asynchronous event delivery
- Event filtering and routing
- Event persistence and replay
- Inter-plugin communication

## Interface

```cpp
class IEventService : public QObject {
    Q_OBJECT

public:
    /// Subscribe to events
    virtual bool subscribe(const QString &eventType,
                          QObject *receiver,
                          const char *slot) = 0;

    /// Unsubscribe from events
    virtual bool unsubscribe(const QString &eventType,
                           QObject *receiver,
                           const char *slot = nullptr) = 0;

    /// Publish event synchronously
    virtual void publish(const QString &eventType,
                        const QVariantMap &data = QVariantMap()) = 0;

    /// Publish event asynchronously
    virtual void publishAsync(const QString &eventType,
                             const QVariantMap &data = QVariantMap()) = 0;

    /// Get event history
    virtual QList<QVariantMap> getEventHistory(const QString &eventType,
                                             int maxEvents = 100) = 0;

    // ... additional methods for event management
};
```

## Event Subscription

### Basic Subscription

```cpp
// Get event service from core
auto eventService = core->getEventService();

if (!eventService) {
    LOG(log, LogLevel::Error, "Event service not available");
    return;
}

// Subscribe to payment events
eventService->subscribe("payment.completed", this, SLOT(onPaymentCompleted));
eventService->subscribe("payment.failed", this, SLOT(onPaymentFailed));
```

### Multiple Event Types

```cpp
// Subscribe to multiple related events
QStringList eventTypes = {"device.connected", "device.disconnected", "device.error"};

for (const QString &eventType : eventTypes) {
    eventService->subscribe(eventType, this, SLOT(onDeviceEvent));
}
```

### Unsubscription

```cpp
// Unsubscribe from specific event
eventService->unsubscribe("payment.completed", this);

// Unsubscribe from all events for this object
eventService->unsubscribe("", this);
```

## Event Publishing

### Synchronous Publishing

```cpp
// Publish payment completion event
QVariantMap paymentData;
paymentData["transactionId"] = transactionId;
paymentData["amount"] = amount;
paymentData["currency"] = "USD";
paymentData["timestamp"] = QDateTime::currentDateTime();

eventService->publish("payment.completed", paymentData);
```

### Asynchronous Publishing

```cpp
// Publish device status event asynchronously
QVariantMap deviceData;
deviceData["deviceId"] = deviceId;
deviceData["status"] = "connected";
deviceData["deviceType"] = "printer";

eventService->publishAsync("device.status.changed", deviceData);
```

## Event Handling

### Event Handler Implementation

```cpp
class PaymentProcessor : public QObject {
    Q_OBJECT

public:
    void processPayment() {
        // Process payment logic...

        // Publish success event
        QVariantMap data;
        data["transactionId"] = "TXN123";
        data["amount"] = 100.50;
        data["status"] = "completed";

        eventService->publish("payment.completed", data);
    }

private slots:
    void onPaymentCompleted(const QVariantMap &data) {
        QString transactionId = data["transactionId"].toString();
        double amount = data["amount"].toDouble();

        LOG(log, LogLevel::Info,
            QString("Payment completed: %1, Amount: %2")
            .arg(transactionId).arg(amount));

        // Update UI, print receipt, etc.
        updateUI();
        printReceipt(transactionId);
    }

    void onPaymentFailed(const QVariantMap &data) {
        QString error = data["error"].toString();
        LOG(log, LogLevel::Error, QString("Payment failed: %1").arg(error));

        // Handle failure
        showErrorMessage(error);
    }
};
```

### Generic Event Handler

```cpp
private slots:
    void onDeviceEvent(const QVariantMap &data) {
        QString eventType = data["eventType"].toString();
        QString deviceId = data["deviceId"].toString();
        QString status = data["status"].toString();

        if (eventType == "device.connected") {
            LOG(log, LogLevel::Info, QString("Device connected: %1").arg(deviceId));
            onDeviceConnected(deviceId);
        } else if (eventType == "device.disconnected") {
            LOG(log, LogLevel::Info, QString("Device disconnected: %1").arg(deviceId));
            onDeviceDisconnected(deviceId);
        } else if (eventType == "device.error") {
            QString error = data["error"].toString();
            LOG(log, LogLevel::Error,
                QString("Device error: %1 - %2").arg(deviceId, error));
            onDeviceError(deviceId, error);
        }
    }
```

## Event History

### Retrieving Event History

```cpp
// Get recent payment events
QList<QVariantMap> paymentHistory =
    eventService->getEventHistory("payment.completed", 50);

// Process historical events
for (const QVariantMap &event : paymentHistory) {
    QString transactionId = event["transactionId"].toString();
    double amount = event["amount"].toDouble();
    QDateTime timestamp = event["timestamp"].toDateTime();

    // Process historical payment
    processHistoricalPayment(transactionId, amount, timestamp);
}
```

### Event Replay

```cpp
// Replay events for recovery
void replayEvents() {
    QList<QVariantMap> events = eventService->getEventHistory("", 1000);

    for (const QVariantMap &event : events) {
        QString eventType = event["eventType"].toString();

        // Replay event based on type
        if (eventType.startsWith("payment.")) {
            replayPaymentEvent(event);
        } else if (eventType.startsWith("device.")) {
            replayDeviceEvent(event);
        }
    }
}
```

## Usage in Plugins

Event Service enables loose coupling between plugins:

```cpp
class PrinterPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mEventService = mCore->getEventService();

            // Subscribe to print requests
            mEventService->subscribe("print.receipt", this, SLOT(onPrintReceipt));
            mEventService->subscribe("print.report", this, SLOT(onPrintReport));
        }

        return true;
    }

    void printReceipt(const QString &receiptData) {
        // Publish print request event
        QVariantMap printData;
        printData["type"] = "receipt";
        printData["data"] = receiptData;
        printData["timestamp"] = QDateTime::currentDateTime();

        mEventService->publishAsync("print.request", printData);
    }

private slots:
    void onPrintReceipt(const QVariantMap &data) {
        QString receiptData = data["data"].toString();

        // Print receipt logic
        if (printToPrinter(receiptData)) {
            LOG(mLog, LogLevel::Info, "Receipt printed successfully");
        } else {
            LOG(mLog, LogLevel::Error, "Failed to print receipt");
        }
    }

private:
    IEventService *mEventService;
    ILog *mLog;
};
```

## Event Filtering

### Custom Event Filters

```cpp
// Subscribe with custom filter
eventService->subscribe("payment.*", this, SLOT(onPaymentEvent));

// Handle filtered events
void onPaymentEvent(const QVariantMap &data) {
    QString eventType = data["eventType"].toString();

    if (eventType == "payment.completed") {
        // Handle completion
    } else if (eventType == "payment.failed") {
        // Handle failure
    } else if (eventType == "payment.refunded") {
        // Handle refund
    }
}
```

## Error Handling

```cpp
try {
    eventService->subscribe("payment.completed", this, SLOT(onPaymentCompleted));

    // Publish event
    QVariantMap data;
    data["transactionId"] = "TXN123";
    eventService->publish("payment.completed", data);

} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Event service error: %1").arg(e.what()));
}
```

## Thread Safety

Event publishing is thread-safe. Events can be published from any thread and will be delivered to subscribers in their respective threads.

## Event Persistence

The Event Service can persist events for recovery:

```cpp
// Events are automatically persisted based on configuration
// Critical events are stored for replay after system restart

// Check if event persistence is enabled
bool persistenceEnabled = settings->getValue("EventService/PersistenceEnabled", true).toBool();
```

## Performance Considerations

- Use asynchronous publishing for non-critical events
- Avoid publishing events in tight loops
- Consider event filtering to reduce processing overhead
- Use appropriate event history limits

## Dependencies

- Settings Service (for event configuration)
- Logging Service (for event logging)

## See Also

- [Settings Service](settings.md) - Event configuration
- [Database Service](database.md) - Event persistence storage
- [Network Service](network.md) - Remote event distribution
