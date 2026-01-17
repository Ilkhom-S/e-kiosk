# Plugin Service

The Plugin Service manages plugin loading, lifecycle, and inter-plugin communication for the EKiosk system.

## Overview

The Plugin Service (`IPluginService`) handles:

- Plugin discovery and loading
- Plugin lifecycle management
- Plugin dependency resolution
- Inter-plugin communication
- Plugin configuration and settings
- Plugin security and sandboxing
- Plugin update and hot-reloading

## Interface

```cpp
class IPluginService : public QObject {
    Q_OBJECT

public:
    enum PluginState { Unloaded, Loading, Loaded, Initializing, Running, Stopping, Error };
    enum PluginType { Core, UI, Payment, Hardware, Utility, Custom };

    struct PluginInfo {
        QString id;
        QString name;
        QString version;
        QString description;
        PluginType type;
        PluginState state;
        QStringList dependencies;
        QStringList providedInterfaces;
        QVariantMap metadata;
        QDateTime loadTime;
        QString errorMessage;
    };

    struct PluginInterface {
        QString interfaceId;
        QString pluginId;
        QObject *instance;
        QVariantMap properties;
    };

    /// Load a plugin by ID
    virtual bool loadPlugin(const QString &pluginId) = 0;

    /// Unload a plugin
    virtual bool unloadPlugin(const QString &pluginId) = 0;

    /// Get plugin information
    virtual PluginInfo getPluginInfo(const QString &pluginId) const = 0;

    /// Get all loaded plugins
    virtual QList<PluginInfo> getLoadedPlugins() const = 0;

    /// Check if plugin is loaded
    virtual bool isPluginLoaded(const QString &pluginId) const = 0;

    /// Get plugin interface
    virtual PluginInterface getPluginInterface(const QString &interfaceId) const = 0;

    /// Register plugin interface
    virtual bool registerInterface(const QString &interfaceId, QObject *instance,
                                  const QVariantMap &properties = {}) = 0;

    /// Unregister plugin interface
    virtual bool unregisterInterface(const QString &interfaceId) = 0;

    /// Send message to plugin
    virtual bool sendPluginMessage(const QString &pluginId, const QString &message,
                                  const QVariantMap &data = {}) = 0;

    /// Broadcast message to all plugins
    virtual void broadcastMessage(const QString &message, const QVariantMap &data = {}) = 0;

    // ... additional methods for plugin management
};
```

## Plugin Loading

### Loading Plugins

```cpp
// Get plugin service from core
auto pluginService = core->getPluginService();

if (!pluginService) {
    LOG(log, LogLevel::Error, "Plugin service not available");
    return;
}

// Load a specific plugin
bool loaded = pluginService->loadPlugin("payment.stripe");

if (loaded) {
    LOG(log, LogLevel::Info, "Stripe payment plugin loaded");
} else {
    LOG(log, LogLevel::Error, "Failed to load Stripe payment plugin");
}
```

### Loading Multiple Plugins

```cpp
void loadRequiredPlugins() {
    QStringList requiredPlugins = {
        "ui.main",
        "payment.processor",
        "hardware.printer",
        "database.sqlite"
    };

    foreach (const QString &pluginId, requiredPlugins) {
        bool loaded = pluginService->loadPlugin(pluginId);

        if (loaded) {
            LOG(log, LogLevel::Info, QString("Plugin loaded: %1").arg(pluginId));
        } else {
            LOG(log, LogLevel::Error, QString("Failed to load plugin: %1").arg(pluginId));
        }
    }
}
```

### Plugin Information

```cpp
void displayPluginInformation() {
    QList<IPluginService::PluginInfo> plugins = pluginService->getLoadedPlugins();

    foreach (const auto &plugin, plugins) {
        QString stateStr;

        switch (plugin.state) {
            case IPluginService::Unloaded: stateStr = "Unloaded"; break;
            case IPluginService::Loading: stateStr = "Loading"; break;
            case IPluginService::Loaded: stateStr = "Loaded"; break;
            case IPluginService::Initializing: stateStr = "Initializing"; break;
            case IPluginService::Running: stateStr = "Running"; break;
            case IPluginService::Stopping: stateStr = "Stopping"; break;
            case IPluginService::Error: stateStr = "Error"; break;
        }

        LOG(log, LogLevel::Info, QString("Plugin: %1 v%2 - State: %3")
            .arg(plugin.name, plugin.version, stateStr));

        if (!plugin.dependencies.isEmpty()) {
            LOG(log, LogLevel::Info, QString("  Dependencies: %1").arg(plugin.dependencies.join(", ")));
        }

        if (!plugin.errorMessage.isEmpty()) {
            LOG(log, LogLevel::Error, QString("  Error: %1").arg(plugin.errorMessage));
        }
    }
}
```

## Plugin Interfaces

### Registering Interfaces

```cpp
void registerPluginInterfaces() {
    // Register payment processing interface
    bool registered = pluginService->registerInterface(
        "IPaymentProcessor",
        this,
        QVariantMap{
            {"supportedMethods", QStringList{"credit_card", "debit_card", "digital_wallet"}},
            {"version", "2.1"},
            {"capabilities", QStringList{"refund", "void", "partial_capture"}}
        }
    );

    if (registered) {
        LOG(log, LogLevel::Info, "Payment processor interface registered");
    } else {
        LOG(log, LogLevel::Error, "Failed to register payment processor interface");
    }

    // Register data export interface
    registered = pluginService->registerInterface(
        "IDataExporter",
        m_dataExporter,
        QVariantMap{
            {"supportedFormats", QStringList{"csv", "json", "xml"}},
            {"maxRecords", 10000}
        }
    );

    if (registered) {
        LOG(log, LogLevel::Info, "Data exporter interface registered");
    }
}
```

### Accessing Interfaces

```cpp
void accessPluginInterface() {
    // Get payment processor interface
    IPluginService::PluginInterface paymentInterface = pluginService->getPluginInterface("IPaymentProcessor");

    if (paymentInterface.instance) {
        LOG(log, LogLevel::Info, QString("Payment interface provided by: %1").arg(paymentInterface.pluginId));

        // Use the interface
        IPaymentProcessor *processor = qobject_cast<IPaymentProcessor*>(paymentInterface.instance);

        if (processor) {
            // Process payment using the interface
            bool success = processor->processPayment(paymentData);
            // ...
        }

    } else {
        LOG(log, LogLevel::Error, "Payment processor interface not available");
    }
}
```

## Inter-Plugin Communication

### Sending Messages

```cpp
void sendPluginMessage() {
    // Send message to specific plugin
    QVariantMap messageData = {
        {"action", "update_config"},
        {"configKey", "timeout"},
        {"configValue", 30}
    };

    bool sent = pluginService->sendPluginMessage("network.client", "config_update", messageData);

    if (sent) {
        LOG(log, LogLevel::Info, "Configuration update message sent to network plugin");
    } else {
        LOG(log, LogLevel::Error, "Failed to send message to network plugin");
    }
}

void broadcastSystemEvent() {
    // Broadcast system event to all plugins
    QVariantMap eventData = {
        {"eventType", "system_startup"},
        {"timestamp", QDateTime::currentDateTime()},
        {"version", getSystemVersion()}
    };

    pluginService->broadcastMessage("system_event", eventData);

    LOG(log, LogLevel::Info, "System startup event broadcasted to all plugins");
}
```

### Receiving Messages

```cpp
void setupMessageHandling() {
    // Connect to message reception signal
    connect(pluginService, &IPluginService::messageReceived,
            this, &MyPlugin::handlePluginMessage);
}

void handlePluginMessage(const QString &senderId, const QString &message, const QVariantMap &data) {
    LOG(log, LogLevel::Info, QString("Received message from %1: %2").arg(senderId, message));

    if (message == "payment_completed") {
        handlePaymentCompleted(data);
    } else if (message == "user_authenticated") {
        handleUserAuthenticated(data);
    } else if (message == "config_changed") {
        handleConfigurationChanged(data);
    } else if (message == "system_shutdown") {
        handleSystemShutdown(data);
    } else {
        LOG(log, LogLevel::Warning, QString("Unknown message type: %1").arg(message));
    }
}

void handlePaymentCompleted(const QVariantMap &data) {
    QString transactionId = data.value("transactionId").toString();
    double amount = data.value("amount", 0.0).toDouble();

    LOG(log, LogLevel::Info, QString("Payment completed: %1 ($%2)").arg(transactionId).arg(amount));

    // Update local state
    updateTransactionStatus(transactionId, "completed");

    // Send receipt
    sendReceipt(transactionId);
}

void handleConfigurationChanged(const QVariantMap &data) {
    QString configKey = data.value("configKey").toString();
    QVariant configValue = data.value("configValue");

    LOG(log, LogLevel::Info, QString("Configuration changed: %1 = %2").arg(configKey, configValue.toString()));

    // Update local configuration
    updateLocalConfig(configKey, configValue);

    // Reinitialize if necessary
    if (configKey == "database.connection") {
        reinitializeDatabaseConnection();
    }
}
```

## Plugin Lifecycle Management

### Plugin State Monitoring

```cpp
void monitorPluginStates() {
    QList<IPluginService::PluginInfo> plugins = pluginService->getLoadedPlugins();

    foreach (const auto &plugin, plugins) {
        switch (plugin.state) {
            case IPluginService::Error:
                LOG(log, LogLevel::Error, QString("Plugin in error state: %1 - %2")
                    .arg(plugin.name, plugin.errorMessage));

                // Attempt recovery
                attemptPluginRecovery(plugin.id);
                break;

            case IPluginService::Unloaded:
                LOG(log, LogLevel::Warning, QString("Plugin unloaded: %1").arg(plugin.name));

                // Check if plugin should be reloaded
                if (shouldReloadPlugin(plugin.id)) {
                    pluginService->loadPlugin(plugin.id);
                }
                break;

            case IPluginService::Running:
                // Plugin is healthy
                break;

            default:
                LOG(log, LogLevel::Info, QString("Plugin %1 state: %2").arg(plugin.name).arg(static_cast<int>(plugin.state)));
                break;
        }
    }
}

void attemptPluginRecovery(const QString &pluginId) {
    LOG(log, LogLevel::Info, QString("Attempting to recover plugin: %1").arg(pluginId));

    // Unload and reload plugin
    pluginService->unloadPlugin(pluginId);
    QThread::msleep(1000); // Wait 1 second

    bool reloaded = pluginService->loadPlugin(pluginId);

    if (reloaded) {
        LOG(log, LogLevel::Info, QString("Plugin recovered: %1").arg(pluginId));
    } else {
        LOG(log, LogLevel::Error, QString("Failed to recover plugin: %1").arg(pluginId));
    }
}
```

### Plugin Dependencies

```cpp
void checkPluginDependencies() {
    QList<IPluginService::PluginInfo> plugins = pluginService->getLoadedPlugins();

    foreach (const auto &plugin, plugins) {
        foreach (const QString &dependency, plugin.dependencies) {
            if (!pluginService->isPluginLoaded(dependency)) {
                LOG(log, LogLevel::Warning, QString("Plugin %1 missing dependency: %2")
                    .arg(plugin.name, dependency));

                // Attempt to load dependency
                bool loaded = pluginService->loadPlugin(dependency);

                if (loaded) {
                    LOG(log, LogLevel::Info, QString("Dependency loaded: %1").arg(dependency));
                } else {
                    LOG(log, LogLevel::Error, QString("Failed to load dependency: %1").arg(dependency));
                }
            }
        }
    }
}
```

## Usage in Plugins

Plugin Service enables plugins to interact with each other and the system:

```cpp
class PaymentPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mPluginService = mCore->getPluginService();
            mDatabaseService = mCore->getDatabaseService();
            mEventService = mCore->getEventService();
            mLog = kernel->getLog("PaymentPlugin");
        }

        return true;
    }

    bool start() override {
        // Register payment interfaces
        registerPaymentInterfaces();

        // Set up inter-plugin communication
        setupPluginCommunication();

        // Load dependent plugins
        loadDependentPlugins();

        return true;
    }

    void registerPaymentInterfaces() {
        // Register payment processor interface
        bool registered = mPluginService->registerInterface(
            "IPaymentProcessor",
            this,
            QVariantMap{
                {"supportedMethods", QStringList{"credit_card", "debit_card"}},
                {"version", "2.0"},
                {"capabilities", QStringList{"auth", "capture", "refund", "void"}}
            }
        );

        if (registered) {
            LOG(mLog, LogLevel::Info, "Payment processor interface registered");
        }

        // Register transaction logger interface
        registered = mPluginService->registerInterface(
            "ITransactionLogger",
            m_transactionLogger,
            QVariantMap{
                {"logLevel", "detailed"},
                {"retentionDays", 90}
            }
        );

        if (registered) {
            LOG(mLog, LogLevel::Info, "Transaction logger interface registered");
        }
    }

    void setupPluginCommunication() {
        // Connect to plugin messages
        connect(mPluginService, &IPluginService::messageReceived,
                this, &PaymentPlugin::handlePluginMessage);

        // Subscribe to relevant events
        mEventService->subscribe("transaction.initiated", [this](const QVariantMap &eventData) {
            handleTransactionInitiated(eventData);
        });

        mEventService->subscribe("user.authenticated", [this](const QVariantMap &eventData) {
            handleUserAuthenticated(eventData);
        });
    }

    void loadDependentPlugins() {
        // Load required plugins
        QStringList dependencies = {"database.storage", "crypto.security", "network.communication"};

        foreach (const QString &pluginId, dependencies) {
            if (!mPluginService->isPluginLoaded(pluginId)) {
                bool loaded = mPluginService->loadPlugin(pluginId);

                if (loaded) {
                    LOG(mLog, LogLevel::Info, QString("Dependency loaded: %1").arg(pluginId));
                } else {
                    LOG(mLog, LogLevel::Warning, QString("Failed to load dependency: %1").arg(pluginId));
                }
            }
        }
    }

    void handlePluginMessage(const QString &senderId, const QString &message, const QVariantMap &data) {
        LOG(mLog, LogLevel::Info, QString("Received message from %1: %2").arg(senderId, message));

        if (message == "process_payment") {
            processPaymentFromPlugin(data);
        } else if (message == "refund_request") {
            processRefundFromPlugin(data);
        } else if (message == "config_update") {
            handleConfigUpdate(data);
        } else if (message == "health_check") {
            respondToHealthCheck(senderId);
        }
    }

    void processPaymentFromPlugin(const QVariantMap &data) {
        QString transactionId = data.value("transactionId").toString();
        double amount = data.value("amount", 0.0).toDouble();
        QString paymentMethod = data.value("paymentMethod").toString();

        LOG(mLog, LogLevel::Info, QString("Processing payment from plugin: %1 ($%2)")
            .arg(transactionId).arg(amount));

        try {
            // Process payment
            QVariantMap paymentResult = processPayment(amount, paymentMethod);

            // Send result back to requesting plugin
            QVariantMap responseData = {
                {"transactionId", transactionId},
                {"success", paymentResult.value("success", false)},
                {"result", paymentResult}
            };

            mPluginService->sendPluginMessage(data.value("requestingPlugin").toString(),
                                             "payment_result", responseData);

        } catch (const std::exception &e) {
            LOG(mLog, LogLevel::Error, QString("Payment processing failed: %1").arg(e.what()));

            // Send error response
            QVariantMap errorData = {
                {"transactionId", transactionId},
                {"success", false},
                {"error", e.what()}
            };

            mPluginService->sendPluginMessage(data.value("requestingPlugin").toString(),
                                             "payment_result", errorData);
        }
    }

    void handleTransactionInitiated(const QVariantMap &eventData) {
        QString transactionId = eventData.value("transactionId").toString();
        QString userId = eventData.value("userId").toString();

        LOG(mLog, LogLevel::Info, QString("Transaction initiated: %1 for user %2")
            .arg(transactionId, userId));

        // Prepare payment processing
        initializePaymentSession(transactionId, userId);

        // Notify other plugins
        mPluginService->broadcastMessage("payment_session_started", {
            {"transactionId", transactionId},
            {"userId", userId}
        });
    }

    void handleUserAuthenticated(const QVariantMap &eventData) {
        QString userId = eventData.value("userId").toString();
        QString sessionId = eventData.value("sessionId").toString();

        LOG(mLog, LogLevel::Info, QString("User authenticated: %1").arg(userId));

        // Load user payment preferences
        loadUserPaymentPreferences(userId);

        // Update payment interface
        updatePaymentInterfaceForUser(userId);
    }

    void handleConfigUpdate(const QVariantMap &data) {
        QString configKey = data.value("configKey").toString();
        QVariant configValue = data.value("configValue");

        LOG(mLog, LogLevel::Info, QString("Configuration updated: %1 = %2")
            .arg(configKey, configValue.toString()));

        // Update local configuration
        if (configKey == "payment.timeout") {
            m_paymentTimeout = configValue.toInt();
        } else if (configKey == "payment.retryCount") {
            m_retryCount = configValue.toInt();
        }

        // Reinitialize payment processor if necessary
        if (configKey.startsWith("payment.processor")) {
            reinitializePaymentProcessor();
        }
    }

    void respondToHealthCheck(const QString &requestingPlugin) {
        // Perform health check
        bool healthy = performHealthCheck();

        QVariantMap healthData = {
            {"pluginId", "payment.processor"},
            {"healthy", healthy},
            {"timestamp", QDateTime::currentDateTime()},
            {"version", "2.0"},
            {"activeTransactions", getActiveTransactionCount()}
        };

        if (!healthy) {
            healthData["issues"] = getHealthIssues();
        }

        // Send health response
        mPluginService->sendPluginMessage(requestingPlugin, "health_response", healthData);

        LOG(mLog, LogLevel::Info, QString("Health check response sent to %1: %2")
            .arg(requestingPlugin, healthy ? "healthy" : "unhealthy"));
    }

    void requestDatabaseAccess() {
        // Request database interface from database plugin
        IPluginService::PluginInterface dbInterface = mPluginService->getPluginInterface("IDatabase");

        if (dbInterface.instance) {
            IDatabase *database = qobject_cast<IDatabase*>(dbInterface.instance);

            if (database) {
                // Use database interface
                QVariantList transactions = database->query("SELECT * FROM transactions WHERE status = 'pending'");

                LOG(mLog, LogLevel::Info, QString("Retrieved %1 pending transactions").arg(transactions.size()));
            }
        } else {
            LOG(mLog, LogLevel::Warning, "Database interface not available");
        }
    }

    void coordinateWithPrinterPlugin() {
        // Check if printer plugin is available
        if (mPluginService->isPluginLoaded("printer.receipt")) {
            // Send receipt printing request
            QVariantMap printData = {
                {"transactionId", m_currentTransactionId},
                {"amount", m_currentAmount},
                {"items", m_currentItems},
                {"timestamp", QDateTime::currentDateTime()}
            };

            bool sent = mPluginService->sendPluginMessage("printer.receipt", "print_receipt", printData);

            if (sent) {
                LOG(mLog, LogLevel::Info, "Receipt printing request sent");
            } else {
                LOG(mLog, LogLevel::Error, "Failed to send receipt printing request");
            }
        } else {
            LOG(mLog, LogLevel::Warning, "Printer plugin not available, skipping receipt printing");
        }
    }

    void handlePluginStateChange(const QString &pluginId, IPluginService::PluginState newState) {
        LOG(mLog, LogLevel::Info, QString("Plugin state changed: %1 -> %2")
            .arg(pluginId).arg(static_cast<int>(newState)));

        if (newState == IPluginService::Error) {
            // Handle plugin error
            if (pluginId == "database.storage") {
                // Database plugin failed, switch to offline mode
                switchToOfflineMode();
            } else if (pluginId == "network.communication") {
                // Network plugin failed, queue transactions
                queueTransactionsForLater();
            }
        } else if (newState == IPluginService::Running) {
            // Plugin recovered
            if (pluginId == "database.storage") {
                // Database back online, sync queued data
                syncQueuedData();
            }
        }
    }

private:
    IPluginService *mPluginService;
    IDatabaseService *mDatabaseService;
    IEventService *mEventService;
    ILog *mLog;

    QObject *m_transactionLogger;
    QString m_currentTransactionId;
    double m_currentAmount;
    QVariantList m_currentItems;
    int m_paymentTimeout;
    int m_retryCount;
};
```

## Error Handling

```cpp
try {
    // Validate plugin parameters
    if (pluginId.isEmpty()) {
        throw std::invalid_argument("Plugin ID cannot be empty");
    }

    // Check plugin service availability
    if (!pluginService) {
        throw std::runtime_error("Plugin service not available");
    }

    // Load plugin
    if (!pluginService->loadPlugin(pluginId)) {
        throw std::runtime_error("Failed to load plugin");
    }

} catch (const std::invalid_argument &e) {
    LOG(log, LogLevel::Error, QString("Invalid plugin operation: %1").arg(e.what()));

} catch (const std::runtime_error &e) {
    LOG(log, LogLevel::Error, QString("Plugin service error: %1").arg(e.what()));

    // Handle error - try alternative plugin, show error message, etc.
    handlePluginError(pluginId, e.what());

} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Unexpected plugin error: %1").arg(e.what()));
}
```

## Plugin Security

```cpp
void validatePluginAccess(const QString &pluginId, const QString &requestedInterface) {
    // Check if plugin is authorized to access the interface
    if (!isPluginAuthorized(pluginId, requestedInterface)) {
        LOG(log, LogLevel::Warning, QString("Plugin %1 not authorized for interface %2")
            .arg(pluginId, requestedInterface));

        throw std::runtime_error("Plugin access denied");
    }

    // Check plugin state
    IPluginService::PluginInfo info = pluginService->getPluginInfo(pluginId);

    if (info.state != IPluginService::Running) {
        LOG(log, LogLevel::Warning, QString("Plugin %1 not in running state").arg(pluginId));
        throw std::runtime_error("Plugin not available");
    }
}

void sandboxPluginExecution(const QString &pluginId) {
    // Set up plugin sandbox
    // - Limit resource access
    // - Monitor execution time
    // - Restrict network access
    // - Control file system access

    LOG(log, LogLevel::Info, QString("Plugin sandbox configured for: %1").arg(pluginId));
}
```

## Performance Considerations

- Load plugins asynchronously to avoid blocking
- Cache plugin interfaces to reduce lookup overhead
- Implement plugin execution timeouts
- Monitor plugin resource usage
- Use lazy loading for non-essential plugins

## Security Considerations

- Validate plugin signatures and integrity
- Implement plugin permission system
- Sandbox plugin execution environment
- Audit plugin communications
- Secure plugin configuration storage

## Dependencies

- Settings Service (for plugin configuration)
- Event Service (for plugin event notifications)
- Security Service (for plugin authentication)
- Log Service (for plugin activity logging)

## See Also

- [Settings Service](settings.md) - Plugin configuration
- [Event Service](event.md) - Plugin event notifications
- [Security Service](../../modules/Crypt/) - Plugin authentication
