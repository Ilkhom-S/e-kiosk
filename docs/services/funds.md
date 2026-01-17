# Funds Service

The Funds Service manages cash handling operations for the EKiosk system.

## Overview

The Funds Service (`IFundsService`) handles:

- Cash acceptance and dispensing
- Coin and bill validation
- Cash drawer management
- Float management and reconciliation
- Cash transaction logging

## Interface

```cpp
class IFundsService : public QObject {
    Q_OBJECT

public:
    enum CashDeviceStatus { Connected, Disconnected, Error, Busy };

    /// Get cash device status
    virtual CashDeviceStatus getStatus() const = 0;

    /// Accept cash payment
    virtual bool acceptCash(double amount, const QString &currency = "USD") = 0;

    /// Dispense cash
    virtual bool dispenseCash(double amount, const QString &currency = "USD") = 0;

    /// Get current cash float
    virtual double getCurrentFloat(const QString &currency = "USD") const = 0;

    /// Add cash to float
    virtual bool addToFloat(double amount, const QString &currency = "USD") = 0;

    /// Remove cash from float
    virtual bool removeFromFloat(double amount, const QString &currency = "USD") = 0;

    /// Open cash drawer
    virtual bool openCashDrawer() = 0;

    /// Get cash transaction history
    virtual QList<QVariantMap> getTransactionHistory(int limit = 100) = 0;

    // ... additional methods for cash management
};
```

## Cash Device Management

### Device Status

```cpp
// Get funds service from core
auto fundsService = core->getFundsService();

if (!fundsService) {
    LOG(log, LogLevel::Error, "Funds service not available");
    return;
}

// Check cash device status
IFundsService::CashDeviceStatus status = fundsService->getStatus();

switch (status) {
    case IFundsService::Connected:
        LOG(log, LogLevel::Info, "Cash device connected");
        enableCashOperations();
        break;
    case IFundsService::Disconnected:
        LOG(log, LogLevel::Warning, "Cash device disconnected");
        disableCashOperations();
        break;
    case IFundsService::Error:
        LOG(log, LogLevel::Error, "Cash device error");
        handleDeviceError();
        break;
    case IFundsService::Busy:
        LOG(log, LogLevel::Info, "Cash device busy");
        break;
}
```

### Cash Drawer Operations

```cpp
// Open cash drawer for manual operations
bool opened = fundsService->openCashDrawer();

if (opened) {
    LOG(log, LogLevel::Info, "Cash drawer opened");
} else {
    LOG(log, LogLevel::Error, "Failed to open cash drawer");
}
```

## Cash Transactions

### Accepting Cash Payments

```cpp
// Accept cash payment
double paymentAmount = 5.00;  // Customer pays $5
bool accepted = fundsService->acceptCash(paymentAmount);

if (accepted) {
    LOG(log, LogLevel::Info, QString("Cash payment accepted: $%1").arg(paymentAmount));

    // Calculate and dispense change if needed
    double totalDue = 2.50;  // Item costs $2.50
    double change = paymentAmount - totalDue;

    if (change > 0) {
        dispenseChange(change);
    }

} else {
    LOG(log, LogLevel::Error, "Cash payment acceptance failed");
}
```

### Dispensing Change

```cpp
bool dispenseChange(double changeAmount) {
    bool dispensed = fundsService->dispenseCash(changeAmount);

    if (dispensed) {
        LOG(log, LogLevel::Info, QString("Change dispensed: $%1").arg(changeAmount));
        return true;
    } else {
        LOG(log, LogLevel::Error, QString("Failed to dispense change: $%1").arg(changeAmount));
        // Handle dispensing failure - manual intervention may be needed
        return false;
    }
}
```

## Float Management

### Checking Current Float

```cpp
// Get current cash float
double currentFloat = fundsService->getCurrentFloat();

LOG(log, LogLevel::Info, QString("Current cash float: $%1").arg(currentFloat));

// Check if float is sufficient for operations
double minimumFloat = settings->getValue("Funds/MinimumFloat", 50.0).toDouble();

if (currentFloat < minimumFloat) {
    LOG(log, LogLevel::Warning, "Cash float is low, refill needed");
    requestFloatRefill();
}
```

### Adding to Float

```cpp
// Add cash to float (e.g., during refill)
double refillAmount = 100.00;
bool added = fundsService->addToFloat(refillAmount);

if (added) {
    LOG(log, LogLevel::Info, QString("Added to float: $%1").arg(refillAmount));

    // Log float refill transaction
    logFloatTransaction("REFILL", refillAmount);
} else {
    LOG(log, LogLevel::Error, "Failed to add cash to float");
}
```

### Removing from Float

```cpp
// Remove cash from float (e.g., bank deposit)
double depositAmount = 200.00;
bool removed = fundsService->removeFromFloat(depositAmount);

if (removed) {
    LOG(log, LogLevel::Info, QString("Removed from float: $%1").arg(depositAmount));
    logFloatTransaction("DEPOSIT", -depositAmount);
} else {
    LOG(log, LogLevel::Error, "Failed to remove cash from float");
}
```

## Transaction History

### Retrieving Transaction History

```cpp
// Get recent cash transactions
QList<QVariantMap> transactions = fundsService->getTransactionHistory(50);

for (const QVariantMap &transaction : transactions) {
    QString type = transaction["type"].toString();
    double amount = transaction["amount"].toDouble();
    QDateTime timestamp = transaction["timestamp"].toDateTime();

    LOG(log, LogLevel::Info,
        QString("Transaction: %1, Amount: $%2, Time: %3")
        .arg(type).arg(amount).arg(timestamp.toString()));
}
```

### Logging Transactions

```cpp
void logFloatTransaction(const QString &type, double amount) {
    QVariantMap transaction;
    transaction["type"] = type;
    transaction["amount"] = amount;
    transaction["timestamp"] = QDateTime::currentDateTime();
    transaction["user"] = currentUser();

    // Store in database
    QVariantMap dbParams;
    dbParams["type"] = type;
    dbParams["amount"] = amount;
    dbParams["timestamp"] = transaction["timestamp"];
    dbParams["user"] = transaction["user"];

    databaseService->executeCommand(
        "INSERT INTO cash_transactions (type, amount, timestamp, user_id) "
        "VALUES (:type, :amount, :timestamp, :user)",
        dbParams
    );

    // Publish transaction event
    eventService->publish("cash.transaction", transaction);
}
```

## Usage in Plugins

Funds Service is commonly used in cash handling plugins:

```cpp
class CashPaymentPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mFundsService = mCore->getFundsService();
            mPaymentService = mCore->getPaymentService();
            mEventService = mCore->getEventService();
            mLog = kernel->getLog("CashPayment");
        }

        return true;
    }

    bool processCashPayment(double totalAmount) {
        if (!mFundsService) {
            LOG(mLog, LogLevel::Error, "Funds service not available");
            return false;
        }

        // Check device status
        if (mFundsService->getStatus() != IFundsService::Connected) {
            LOG(mLog, LogLevel::Error, "Cash device not available");
            return false;
        }

        // Start payment transaction
        QString transactionId = mPaymentService->startPayment(totalAmount);

        if (transactionId.isEmpty()) {
            LOG(mLog, LogLevel::Error, "Failed to start payment transaction");
            return false;
        }

        // Accept cash
        LOG(mLog, LogLevel::Info, QString("Accepting cash payment for: $%1").arg(totalAmount));

        bool accepted = mFundsService->acceptCash(totalAmount);

        if (accepted) {
            // Process payment
            QVariantMap paymentData;
            paymentData["method"] = "cash";
            paymentData["transactionId"] = transactionId;

            bool processed = mPaymentService->processPayment(transactionId, "cash", paymentData);

            if (processed) {
                LOG(mLog, LogLevel::Info, QString("Cash payment completed: %1").arg(transactionId));

                // Publish success event
                QVariantMap eventData;
                eventData["transactionId"] = transactionId;
                eventData["amount"] = totalAmount;
                eventData["method"] = "cash";
                mEventService->publish("payment.cash.completed", eventData);

                return true;
            }
        }

        // Payment failed
        LOG(mLog, LogLevel::Error, QString("Cash payment failed: %1").arg(transactionId));
        mPaymentService->cancelPayment(transactionId);

        return false;
    }

    bool dispenseChange(double changeAmount) {
        if (changeAmount <= 0) return true;

        LOG(mLog, LogLevel::Info, QString("Dispensing change: $%1").arg(changeAmount));

        bool dispensed = mFundsService->dispenseCash(changeAmount);

        if (dispensed) {
            // Log change dispensing
            logCashTransaction("CHANGE_DISPENSED", -changeAmount);
            return true;
        } else {
            LOG(mLog, LogLevel::Error, "Failed to dispense change");
            return false;
        }
    }

private:
    void logCashTransaction(const QString &type, double amount) {
        QVariantMap transaction;
        transaction["type"] = type;
        transaction["amount"] = amount;
        transaction["timestamp"] = QDateTime::currentDateTime();

        // Store transaction
        QVariantMap dbParams;
        dbParams["type"] = type;
        dbParams["amount"] = amount;
        dbParams["timestamp"] = transaction["timestamp"];

        // Assuming database service is available
        // databaseService->executeCommand(...);
    }

private:
    IFundsService *mFundsService;
    IPaymentService *mPaymentService;
    IEventService *mEventService;
    ILog *mLog;
};
```

## Error Handling

```cpp
try {
    // Check device status before operations
    IFundsService::CashDeviceStatus status = fundsService->getStatus();

    if (status != IFundsService::Connected) {
        throw std::runtime_error("Cash device not connected");
    }

    // Attempt cash operation
    if (!fundsService->acceptCash(amount)) {
        throw std::runtime_error("Cash acceptance failed");
    }

} catch (const std::runtime_error &e) {
    LOG(log, LogLevel::Error, QString("Funds service error: %1").arg(e.what()));

    // Handle error - notify user, fallback to other payment methods, etc.
    handleCashError();

} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Unexpected funds error: %1").arg(e.what()));
}
```

## Float Reconciliation

```cpp
// Perform end-of-day float reconciliation
void reconcileFloat() {
    double expectedFloat = calculateExpectedFloat();
    double actualFloat = fundsService->getCurrentFloat();

    double difference = actualFloat - expectedFloat;

    if (qAbs(difference) > 0.01) {  // Allow for small discrepancies
        LOG(log, LogLevel::Warning,
            QString("Float discrepancy detected. Expected: $%1, Actual: $%2, Difference: $%3")
            .arg(expectedFloat).arg(actualFloat).arg(difference));

        // Log discrepancy
        logFloatDiscrepancy(expectedFloat, actualFloat, difference);
    } else {
        LOG(log, LogLevel::Info, "Float reconciliation successful");
    }
}
```

## Security Considerations

- Implement proper audit trails for all cash operations
- Use secure storage for cash devices
- Regular float counts and reconciliations
- Tamper detection and alerts

## Dependencies

- Device Service (for cash device hardware)
- Payment Service (for payment integration)
- Database Service (for transaction storage)
- Event Service (for cash operation notifications)
- Settings Service (for float limits and configuration)

## See Also

- [Device Service](device.md) - Cash device hardware management
- [Payment Service](payment.md) - Payment processing integration
- [Database Service](database.md) - Transaction storage
- [Event Service](event.md) - Cash operation notifications
