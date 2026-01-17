# Hook Service

The Hook Service provides a plugin system for extending and customizing EKiosk functionality through event-driven hooks.

## Overview

The Hook Service (`IHookService`) enables:

- Event-driven plugin extensions
- Customizable system behavior
- Plugin communication and coordination
- Hook registration and management
- Synchronous and asynchronous hook execution
- Hook priority and ordering

## Interface

```cpp
class IHookService : public QObject {
    Q_OBJECT

public:
    enum HookPriority { Lowest = 0, Low = 25, Normal = 50, High = 75, Highest = 100 };
    enum HookResult { Continue, Stop, Error };

    using HookCallback = std::function<HookResult(const QVariantMap &context)>;

    /// Register a hook for an event
    virtual bool registerHook(const QString &eventName, const QString &hookId,
                             HookCallback callback, HookPriority priority = Normal) = 0;

    /// Unregister a hook
    virtual bool unregisterHook(const QString &eventName, const QString &hookId) = 0;

    /// Execute hooks for an event (synchronous)
    virtual HookResult executeHooks(const QString &eventName, const QVariantMap &context) = 0;

    /// Execute hooks asynchronously
    virtual void executeHooksAsync(const QString &eventName, const QVariantMap &context) = 0;

    /// Check if hooks are registered for an event
    virtual bool hasHooks(const QString &eventName) const = 0;

    /// Get registered hook IDs for an event
    virtual QStringList getHookIds(const QString &eventName) const = 0;

    /// Set hook execution timeout
    virtual void setHookTimeout(int milliseconds) = 0;

    /// Enable/disable hook execution
    virtual void setHooksEnabled(bool enabled) = 0;

    // ... additional methods for hook management
};
```

## Hook Registration

### Basic Hook Registration

```cpp
// Get hook service from core
auto hookService = core->getHookService();

if (!hookService) {
    LOG(log, LogLevel::Error, "Hook service not available");
    return;
}

// Register a hook for payment processing
bool registered = hookService->registerHook(
    "payment.before_process",
    "my_plugin.payment_validation",
    [this](const QVariantMap &context) -> IHookService::HookResult {
        return validatePayment(context);
    },
    IHookService::High  // High priority
);

if (registered) {
    LOG(log, LogLevel::Info, "Payment validation hook registered");
} else {
    LOG(log, LogLevel::Error, "Failed to register payment validation hook");
}
```

### Multiple Hook Registration

```cpp
void registerPluginHooks() {
    // Register hooks for different events
    hookService->registerHook("user.login", "auth_plugin.session_init",
        [this](const QVariantMap &context) { return initializeUserSession(context); });

    hookService->registerHook("payment.completed", "loyalty_plugin.points_award",
        [this](const QVariantMap &context) { return awardLoyaltyPoints(context); });

    hookService->registerHook("product.scanned", "inventory_plugin.stock_check",
        [this](const QVariantMap &context) { return checkProductStock(context); });

    hookService->registerHook("transaction.finalize", "audit_plugin.log_transaction",
        [this](const QVariantMap &context) { return logTransaction(context); });

    LOG(log, LogLevel::Info, "Plugin hooks registered successfully");
}
```

## Hook Implementation

### Payment Validation Hook

```cpp
IHookService::HookResult validatePayment(const QVariantMap &context) {
    try {
        // Extract payment data from context
        double amount = context.value("amount", 0.0).toDouble();
        QString paymentMethod = context.value("paymentMethod").toString();
        QString userId = context.value("userId").toString();

        // Validate payment amount
        if (amount <= 0) {
            LOG(log, LogLevel::Error, "Invalid payment amount");
            return IHookService::Error;
        }

        // Check payment method restrictions
        if (!isPaymentMethodAllowed(paymentMethod, userId)) {
            LOG(log, LogLevel::Warning, QString("Payment method not allowed: %1").arg(paymentMethod));
            return IHookService::Stop;  // Stop processing
        }

        // Check user balance/credit
        if (!hasSufficientFunds(userId, amount)) {
            LOG(log, LogLevel::Warning, "Insufficient funds for payment");
            return IHookService::Stop;
        }

        // Additional validation logic
        if (!validatePaymentSecurity(context)) {
            LOG(log, LogLevel::Error, "Payment security validation failed");
            return IHookService::Error;
        }

        LOG(log, LogLevel::Info, QString("Payment validation passed for user: %1").arg(userId));
        return IHookService::Continue;

    } catch (const std::exception &e) {
        LOG(log, LogLevel::Error, QString("Payment validation error: %1").arg(e.what()));
        return IHookService::Error;
    }
}
```

### User Session Hook

```cpp
IHookService::HookResult initializeUserSession(const QVariantMap &context) {
    QString userId = context.value("userId").toString();
    QString sessionId = context.value("sessionId").toString();

    try {
        // Initialize user preferences
        loadUserPreferences(userId);

        // Set up user-specific settings
        configureUserInterface(userId);

        // Initialize shopping cart
        initializeShoppingCart(sessionId);

        // Load loyalty program status
        loadLoyaltyStatus(userId);

        // Send welcome notification
        sendWelcomeNotification(userId);

        LOG(log, LogLevel::Info, QString("User session initialized: %1").arg(userId));
        return IHookService::Continue;

    } catch (const std::exception &e) {
        LOG(log, LogLevel::Error, QString("Failed to initialize user session: %1").arg(e.what()));
        return IHookService::Error;
    }
}
```

### Product Scan Hook

```cpp
IHookService::HookResult checkProductStock(const QVariantMap &context) {
    QString productId = context.value("productId").toString();
    int quantity = context.value("quantity", 1).toInt();

    try {
        // Check product availability
        int availableStock = getProductStock(productId);

        if (availableStock < quantity) {
            LOG(log, LogLevel::Warning,
                QString("Insufficient stock for product %1: requested %2, available %3")
                .arg(productId).arg(quantity).arg(availableStock));

            // Notify user of stock issue
            notifyStockIssue(productId, quantity, availableStock);

            return IHookService::Stop;  // Prevent adding to cart
        }

        // Reserve stock temporarily
        if (!reserveStock(productId, quantity)) {
            LOG(log, LogLevel::Error, QString("Failed to reserve stock for product: %1").arg(productId));
            return IHookService::Error;
        }

        LOG(log, LogLevel::Info, QString("Stock check passed for product: %1").arg(productId));
        return IHookService::Continue;

    } catch (const std::exception &e) {
        LOG(log, LogLevel::Error, QString("Stock check error: %1").arg(e.what()));
        return IHookService::Error;
    }
}
```

## Hook Execution

### Synchronous Execution

```cpp
// Execute hooks synchronously for payment processing
QVariantMap paymentContext = {
    {"amount", 25.99},
    {"paymentMethod", "credit_card"},
    {"userId", "user123"},
    {"transactionId", "txn_001"}
};

IHookService::HookResult result = hookService->executeHooks("payment.before_process", paymentContext);

switch (result) {
    case IHookService::Continue:
        LOG(log, LogLevel::Info, "Payment hooks completed successfully, proceeding with payment");
        processPayment(paymentContext);
        break;

    case IHookService::Stop:
        LOG(log, LogLevel::Warning, "Payment hooks stopped processing");
        showPaymentError("Payment validation failed");
        break;

    case IHookService::Error:
        LOG(log, LogLevel::Error, "Payment hooks encountered an error");
        showPaymentError("Payment processing error");
        break;
}
```

### Asynchronous Execution

```cpp
// Execute hooks asynchronously for non-critical operations
void processUserLogout(const QString &userId) {
    QVariantMap logoutContext = {
        {"userId", userId},
        {"sessionId", getCurrentSessionId()},
        {"logoutTime", QDateTime::currentDateTime()}
    };

    // Execute cleanup hooks asynchronously
    hookService->executeHooksAsync("user.logout", logoutContext);

    // Continue with immediate logout
    performImmediateLogout(userId);
}
```

## Hook Management

### Hook Registration Management

```cpp
void managePluginHooks() {
    // Check if hooks are registered
    if (hookService->hasHooks("payment.completed")) {
        LOG(log, LogLevel::Info, "Payment completion hooks are registered");

        // Get registered hook IDs
        QStringList hookIds = hookService->getHookIds("payment.completed");
        LOG(log, LogLevel::Info, QString("Registered hooks: %1").arg(hookIds.join(", ")));
    }

    // Temporarily disable hooks for maintenance
    hookService->setHooksEnabled(false);

    // Perform maintenance operations
    performSystemMaintenance();

    // Re-enable hooks
    hookService->setHooksEnabled(true);
}
```

### Dynamic Hook Registration

```cpp
void registerConditionalHooks() {
    // Register hooks based on configuration
    auto settings = core->getSettingsService();

    if (settings->getValue("hooks.enableLoyalty", true).toBool()) {
        hookService->registerHook("purchase.completed", "loyalty.award_points",
            [this](const QVariantMap &context) { return awardLoyaltyPoints(context); });
    }

    if (settings->getValue("hooks.enableAudit", true).toBool()) {
        hookService->registerHook("transaction.any", "audit.log_all",
            [this](const QVariantMap &context) { return logTransaction(context); });
    }

    if (settings->getValue("hooks.enableNotifications", true).toBool()) {
        hookService->registerHook("user.action", "notification.user_feedback",
            [this](const QVariantMap &context) { return sendUserNotification(context); });
    }
}
```

## Usage in Plugins

Hook Service enables plugins to extend system functionality:

```cpp
class LoyaltyPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mHookService = mCore->getHookService();
            mDatabaseService = mCore->getDatabaseService();
            mLog = kernel->getLog("LoyaltyPlugin");
        }

        return true;
    }

    bool start() override {
        // Register loyalty hooks
        registerLoyaltyHooks();
        return true;
    }

    void registerLoyaltyHooks() {
        // Hook into purchase completion
        mHookService->registerHook("purchase.completed", "loyalty.points_award",
            [this](const QVariantMap &context) { return awardPurchasePoints(context); },
            IHookService::Normal);

        // Hook into user registration
        mHookService->registerHook("user.registered", "loyalty.welcome_bonus",
            [this](const QVariantMap &context) { return awardWelcomeBonus(context); },
            IHookService::High);

        // Hook into product returns
        mHookService->registerHook("product.returned", "loyalty.points_deduct",
            [this](const QVariantMap &context) { return deductReturnPoints(context); },
            IHookService::Normal);

        LOG(mLog, LogLevel::Info, "Loyalty hooks registered");
    }

    IHookService::HookResult awardPurchasePoints(const QVariantMap &context) {
        QString userId = context.value("userId").toString();
        double purchaseAmount = context.value("totalAmount", 0.0).toDouble();

        try {
            // Calculate points (1 point per $1 spent)
            int pointsEarned = static_cast<int>(purchaseAmount);

            // Get current points balance
            int currentPoints = getUserPoints(userId);

            // Award points
            int newBalance = currentPoints + pointsEarned;
            updateUserPoints(userId, newBalance);

            // Log points award
            logPointsTransaction(userId, "purchase", pointsEarned, newBalance);

            // Notify user
            sendPointsNotification(userId, pointsEarned, newBalance);

            LOG(mLog, LogLevel::Info,
                QString("Awarded %1 points to user %2, new balance: %3")
                .arg(pointsEarned).arg(userId).arg(newBalance));

            return IHookService::Continue;

        } catch (const std::exception &e) {
            LOG(mLog, LogLevel::Error,
                QString("Failed to award points to user %1: %2").arg(userId, e.what()));
            return IHookService::Error;
        }
    }

    IHookService::HookResult awardWelcomeBonus(const QVariantMap &context) {
        QString userId = context.value("userId").toString();

        try {
            // Check if user already received welcome bonus
            if (hasReceivedWelcomeBonus(userId)) {
                LOG(mLog, LogLevel::Info, QString("Welcome bonus already awarded to user: %1").arg(userId));
                return IHookService::Continue;
            }

            // Award welcome bonus
            int bonusPoints = 100;  // Welcome bonus amount
            int currentPoints = getUserPoints(userId);
            int newBalance = currentPoints + bonusPoints;

            updateUserPoints(userId, newBalance);
            markWelcomeBonusReceived(userId);

            logPointsTransaction(userId, "welcome_bonus", bonusPoints, newBalance);
            sendWelcomeBonusNotification(userId, bonusPoints);

            LOG(mLog, LogLevel::Info,
                QString("Welcome bonus awarded to user %1: %2 points").arg(userId).arg(bonusPoints));

            return IHookService::Continue;

        } catch (const std::exception &e) {
            LOG(mLog, LogLevel::Error,
                QString("Failed to award welcome bonus to user %1: %2").arg(userId, e.what()));
            return IHookService::Error;
        }
    }

    IHookService::HookResult deductReturnPoints(const QVariantMap &context) {
        QString userId = context.value("userId").toString();
        double returnAmount = context.value("returnAmount", 0.0).toDouble();

        try {
            // Calculate points to deduct (same rate as earning)
            int pointsDeducted = static_cast<int>(returnAmount);

            int currentPoints = getUserPoints(userId);
            int newBalance = std::max(0, currentPoints - pointsDeducted);  // Don't go negative

            actualDeducted = currentPoints - newBalance;
            updateUserPoints(userId, newBalance);

            logPointsTransaction(userId, "return", -actualDeducted, newBalance);
            sendReturnNotification(userId, actualDeducted, newBalance);

            LOG(mLog, LogLevel::Info,
                QString("Deducted %1 points from user %2 for return, new balance: %3")
                .arg(actualDeducted).arg(userId).arg(newBalance));

            return IHookService::Continue;

        } catch (const std::exception &e) {
            LOG(mLog, LogLevel::Error,
                QString("Failed to deduct return points from user %1: %2").arg(userId, e.what()));
            return IHookService::Error;
        }
    }

    void redeemPoints(const QString &userId, int pointsToRedeem) {
        // Trigger points redemption hook
        QVariantMap redeemContext = {
            {"userId", userId},
            {"pointsToRedeem", pointsToRedeem},
            {"redemptionTime", QDateTime::currentDateTime()}
        };

        IHookService::HookResult result = mHookService->executeHooks("loyalty.points_redeem", redeemContext);

        if (result == IHookService::Continue) {
            LOG(mLog, LogLevel::Info, QString("Points redemption processed for user: %1").arg(userId));
        } else {
            LOG(mLog, LogLevel::Warning, QString("Points redemption failed for user: %1").arg(userId));
        }
    }

private:
    IHookService *mHookService;
    IDatabaseService *mDatabaseService;
    ILog *mLog;

    // Helper methods for points management
    int getUserPoints(const QString &userId) {
        // Database query to get user points
        return mDatabaseService->executeQuery("SELECT points FROM user_loyalty WHERE user_id = ?",
                                             {userId}).value(0).toMap().value("points", 0).toInt();
    }

    void updateUserPoints(const QString &userId, int newBalance) {
        // Update points in database
        mDatabaseService->executeQuery("UPDATE user_loyalty SET points = ? WHERE user_id = ?",
                                      {newBalance, userId});
    }

    void logPointsTransaction(const QString &userId, const QString &type, int points, int balance) {
        // Log transaction to database
        mDatabaseService->executeQuery(
            "INSERT INTO loyalty_transactions (user_id, type, points, balance, timestamp) VALUES (?, ?, ?, ?, ?)",
            {userId, type, points, balance, QDateTime::currentDateTime()});
    }
};
```

## Error Handling

```cpp
try {
    // Validate hook parameters
    if (eventName.isEmpty()) {
        throw std::invalid_argument("Event name cannot be empty");
    }

    if (hookId.isEmpty()) {
        throw std::invalid_argument("Hook ID cannot be empty");
    }

    // Check hook service availability
    if (!hookService) {
        throw std::runtime_error("Hook service not available");
    }

    // Register hook
    if (!hookService->registerHook(eventName, hookId, callback, priority)) {
        throw std::runtime_error("Failed to register hook");
    }

} catch (const std::invalid_argument &e) {
    LOG(log, LogLevel::Error, QString("Invalid hook registration: %1").arg(e.what()));

} catch (const std::runtime_error &e) {
    LOG(log, LogLevel::Error, QString("Hook service error: %1").arg(e.what()));

    // Handle error - retry registration, use fallback, etc.
    handleHookRegistrationError(eventName, hookId);

} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Unexpected hook error: %1").arg(e.what()));
}
```

## Hook Execution Timeout

```cpp
// Set hook execution timeout to prevent hanging
hookService->setHookTimeout(5000);  // 5 seconds timeout

// Execute hooks with timeout protection
IHookService::HookResult result = hookService->executeHooks("payment.process", context);

if (result == IHookService::Error) {
    LOG(log, LogLevel::Warning, "Hook execution timed out or failed");

    // Continue with default behavior
    processPaymentWithDefaultBehavior(context);
}
```

## Performance Considerations

- Use asynchronous execution for non-critical hooks
- Set appropriate timeouts to prevent blocking
- Implement hook prioritization for critical operations
- Cache hook results when possible
- Monitor hook execution performance

## Security Considerations

- Validate hook context data to prevent injection
- Implement hook execution permissions
- Sanitize hook IDs and event names
- Monitor hook execution for anomalies
- Implement hook execution limits

## Hook Events Reference

Common hook events in EKiosk:

- `user.login` - User authentication
- `user.logout` - User session end
- `payment.before_process` - Pre-payment validation
- `payment.completed` - Post-payment processing
- `product.scanned` - Product barcode scan
- `purchase.completed` - Transaction completion
- `inventory.updated` - Stock level changes
- `system.maintenance` - Maintenance operations

## Dependencies

- Event Service (for hook event distribution)
- Settings Service (for hook configuration)
- Database Service (for hook persistence)

## See Also

- [Event Service](event.md) - Hook event distribution
- [Settings Service](settings.md) - Hook configuration
- [Database Service](database.md) - Hook persistence
