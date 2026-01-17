# Payment Service

The Payment Service handles payment processing and transaction management for the EKiosk system.

## Overview

The Payment Service (`IPaymentService`) manages:

- Payment transaction processing
- Multiple payment method support
- Transaction state management
- Payment validation and verification
- Receipt generation and management

## Interface

```cpp
class IPaymentService : public QObject {
    Q_OBJECT

public:
    enum PaymentStatus { Pending, Processing, Completed, Failed, Cancelled, Refunded };

    /// Start payment transaction
    virtual QString startPayment(double amount,
                               const QString &currency = "USD",
                               const QVariantMap &metadata = QVariantMap()) = 0;

    /// Process payment with specific method
    virtual bool processPayment(const QString &transactionId,
                              const QString &paymentMethod,
                              const QVariantMap &paymentData) = 0;

    /// Cancel payment transaction
    virtual bool cancelPayment(const QString &transactionId) = 0;

    /// Refund payment transaction
    virtual bool refundPayment(const QString &transactionId,
                             double refundAmount = 0.0) = 0;

    /// Get payment status
    virtual PaymentStatus getPaymentStatus(const QString &transactionId) = 0;

    /// Get payment details
    virtual QVariantMap getPaymentDetails(const QString &transactionId) = 0;

    /// Get supported payment methods
    virtual QStringList getSupportedPaymentMethods() const = 0;

    // ... additional methods for receipt management
};
```

## Payment Transaction Lifecycle

### Starting a Payment

```cpp
// Get payment service from core
auto paymentService = core->getPaymentService();

if (!paymentService) {
    LOG(log, LogLevel::Error, "Payment service not available");
    return;
}

// Start payment transaction
QVariantMap metadata;
metadata["product"] = "Coffee";
metadata["quantity"] = 1;
metadata["unitPrice"] = 2.50;

QString transactionId = paymentService->startPayment(2.50, "USD", metadata);

if (transactionId.isEmpty()) {
    LOG(log, LogLevel::Error, "Failed to start payment transaction");
    return;
}

LOG(log, LogLevel::Info, QString("Payment started: %1").arg(transactionId));
```

### Processing Payment

```cpp
// Process payment with cash
QVariantMap paymentData;
paymentData["method"] = "cash";
paymentData["amountReceived"] = 5.00;  // Customer paid $5
paymentData["change"] = 2.50;          // Return $2.50 change

bool success = paymentService->processPayment(transactionId, "cash", paymentData);

if (success) {
    LOG(log, LogLevel::Info, QString("Payment processed: %1").arg(transactionId));
    // Update UI, print receipt, etc.
} else {
    LOG(log, LogLevel::Error, QString("Payment processing failed: %1").arg(transactionId));
}
```

### Card Payment Processing

```cpp
// Process payment with card
QVariantMap cardData;
cardData["method"] = "card";
cardData["cardNumber"] = "4111111111111111";  // Masked for security
cardData["expiryMonth"] = 12;
cardData["expiryYear"] = 2025;
cardData["cvv"] = "123";

bool cardSuccess = paymentService->processPayment(transactionId, "card", cardData);

if (cardSuccess) {
    LOG(log, LogLevel::Info, "Card payment approved");
} else {
    LOG(log, LogLevel::Error, "Card payment declined");
}
```

## Payment Status Management

### Checking Payment Status

```cpp
// Check payment status
IPaymentService::PaymentStatus status = paymentService->getPaymentStatus(transactionId);

switch (status) {
    case IPaymentService::Pending:
        LOG(log, LogLevel::Info, "Payment is pending");
        break;
    case IPaymentService::Processing:
        LOG(log, LogLevel::Info, "Payment is being processed");
        break;
    case IPaymentService::Completed:
        LOG(log, LogLevel::Info, "Payment completed successfully");
        onPaymentCompleted(transactionId);
        break;
    case IPaymentService::Failed:
        LOG(log, LogLevel::Error, "Payment failed");
        onPaymentFailed(transactionId);
        break;
    case IPaymentService::Cancelled:
        LOG(log, LogLevel::Info, "Payment was cancelled");
        break;
    case IPaymentService::Refunded:
        LOG(log, LogLevel::Info, "Payment was refunded");
        break;
}
```

### Getting Payment Details

```cpp
// Get detailed payment information
QVariantMap details = paymentService->getPaymentDetails(transactionId);

if (!details.isEmpty()) {
    double amount = details["amount"].toDouble();
    QString currency = details["currency"].toString();
    QString method = details["paymentMethod"].toString();
    QDateTime timestamp = details["timestamp"].toDateTime();

    LOG(log, LogLevel::Info,
        QString("Payment details - Amount: %1 %2, Method: %3, Time: %4")
        .arg(amount).arg(currency).arg(method).arg(timestamp.toString()));
}
```

## Payment Methods

### Supported Payment Methods

```cpp
// Get available payment methods
QStringList methods = paymentService->getSupportedPaymentMethods();

LOG(log, LogLevel::Info, QString("Supported payment methods: %1").arg(methods.join(", ")));

// Check if specific method is supported
if (methods.contains("card")) {
    // Enable card payment option
    enableCardPayment();
}

if (methods.contains("cash")) {
    // Enable cash payment option
    enableCashPayment();
}
```

### Custom Payment Methods

```cpp
// Process payment with custom method (e.g., mobile wallet)
QVariantMap walletData;
walletData["method"] = "mobile_wallet";
walletData["walletId"] = "user123";
walletData["phoneNumber"] = "+1234567890";

bool walletSuccess = paymentService->processPayment(transactionId, "mobile_wallet", walletData);
```

## Transaction Management

### Cancelling Payments

```cpp
// Cancel pending payment
bool cancelled = paymentService->cancelPayment(transactionId);

if (cancelled) {
    LOG(log, LogLevel::Info, QString("Payment cancelled: %1").arg(transactionId));
} else {
    LOG(log, LogLevel::Error, QString("Failed to cancel payment: %1").arg(transactionId));
}
```

### Processing Refunds

```cpp
// Refund full payment
bool refunded = paymentService->refundPayment(transactionId);

if (refunded) {
    LOG(log, LogLevel::Info, QString("Payment refunded: %1").arg(transactionId));
}

// Partial refund
bool partialRefund = paymentService->refundPayment(transactionId, 1.25); // Refund $1.25

if (partialRefund) {
    LOG(log, LogLevel::Info, QString("Partial refund processed: %1").arg(transactionId));
}
```

## Usage in Plugins

Payment Service is commonly used in payment processing plugins:

```cpp
class PaymentHandler : public QObject {
    Q_OBJECT

public:
    PaymentHandler(IPaymentService *paymentService, ILog *log)
        : mPaymentService(paymentService), mLog(log) {}

    QString initiatePayment(double amount, const QString &product) {
        QVariantMap metadata;
        metadata["product"] = product;
        metadata["timestamp"] = QDateTime::currentDateTime();

        QString transactionId = mPaymentService->startPayment(amount, "USD", metadata);

        if (!transactionId.isEmpty()) {
            LOG(mLog, LogLevel::Info,
                QString("Payment initiated: %1 for %2").arg(transactionId, product));
        }

        return transactionId;
    }

    bool completeCashPayment(const QString &transactionId, double amountReceived) {
        QVariantMap paymentData;
        paymentData["method"] = "cash";
        paymentData["amountReceived"] = amountReceived;

        // Calculate change
        QVariantMap details = mPaymentService->getPaymentDetails(transactionId);
        double totalAmount = details["amount"].toDouble();
        paymentData["change"] = amountReceived - totalAmount;

        bool success = mPaymentService->processPayment(transactionId, "cash", paymentData);

        if (success) {
            LOG(mLog, LogLevel::Info,
                QString("Cash payment completed: %1").arg(transactionId));
        }

        return success;
    }

    void checkPaymentStatus(const QString &transactionId) {
        IPaymentService::PaymentStatus status = mPaymentService->getPaymentStatus(transactionId);

        switch (status) {
            case IPaymentService::Completed:
                onPaymentSuccess(transactionId);
                break;
            case IPaymentService::Failed:
                onPaymentFailure(transactionId);
                break;
            default:
                // Still processing
                break;
        }
    }

private:
    void onPaymentSuccess(const QString &transactionId) {
        // Generate receipt
        generateReceipt(transactionId);

        // Update inventory
        updateInventory(transactionId);

        // Notify other systems
        emit paymentCompleted(transactionId);
    }

    void onPaymentFailure(const QString &transactionId) {
        LOG(mLog, LogLevel::Error, QString("Payment failed: %1").arg(transactionId));

        // Handle failure (refund, notify user, etc.)
        emit paymentFailed(transactionId);
    }

private:
    IPaymentService *mPaymentService;
    ILog *mLog;
};
```

## Error Handling

```cpp
try {
    QString transactionId = paymentService->startPayment(10.00);

    if (transactionId.isEmpty()) {
        throw std::runtime_error("Failed to start payment transaction");
    }

    // Process payment
    QVariantMap paymentData;
    paymentData["method"] = "card";
    // ... card data ...

    if (!paymentService->processPayment(transactionId, "card", paymentData)) {
        throw std::runtime_error("Payment processing failed");
    }

} catch (const std::runtime_error &e) {
    LOG(log, LogLevel::Error, QString("Payment error: %1").arg(e.what()));
    // Handle error - notify user, rollback, etc.
} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Unexpected payment error: %1").arg(e.what()));
}
```

## Receipt Management

```cpp
// Generate receipt after successful payment
void generateReceipt(const QString &transactionId) {
    QVariantMap details = paymentService->getPaymentDetails(transactionId);

    // Create receipt data
    QVariantMap receiptData;
    receiptData["transactionId"] = transactionId;
    receiptData["amount"] = details["amount"];
    receiptData["currency"] = details["currency"];
    receiptData["method"] = details["paymentMethod"];
    receiptData["timestamp"] = details["timestamp"];

    // Publish receipt generation event
    eventService->publish("receipt.generate", receiptData);
}
```

## Security Considerations

- Never log sensitive payment data (card numbers, CVV, etc.)
- Use secure communication for payment processing
- Validate payment amounts and currencies
- Implement proper audit trails

## Dependencies

- Crypt Service (for payment data encryption)
- Database Service (for transaction storage)
- Event Service (for payment notifications)
- Settings Service (for payment configuration)

## See Also

- [Crypt Service](crypt.md) - Payment data encryption
- [Database Service](database.md) - Transaction storage
- [Event Service](event.md) - Payment notifications
- [Funds Service](funds.md) - Cash handling integration
