# Network Service

The Network Service manages network connectivity and HTTP communications for EKiosk.

## Overview

The Network Service (`INetworkService`) provides:

- Connection establishment and testing
- Network task management via `NetworkTaskManager`
- Connection parameter management
- Error handling and status monitoring

## Interface

```cpp
class INetworkService {
public:
    /// Establish connection
    virtual bool openConnection(bool aWait = true) = 0;

    /// Close connection
    virtual bool closeConnection() = 0;

    /// Check connection status
    virtual bool isConnected(bool aUseCache = false) = 0;

    /// Test connection (open, test, close)
    virtual bool testConnection() = 0;

    /// Get network task manager
    virtual NetworkTaskManager *getNetworkTaskManager() = 0;

    /// Set connection parameters
    virtual void setConnection(const SDK::PaymentProcessor::SConnection &aConnection) = 0;

    /// Get connection parameters
    virtual SDK::PaymentProcessor::SConnection getConnection() const = 0;

    /// Get last connection error
    virtual QString getLastConnectionError() const = 0;
};
```

## Connection Management

### Establishing Connections

```cpp
auto networkService = core->getNetworkService();

// Open connection (blocking)
bool connected = networkService->openConnection(true);

// Open connection (non-blocking)
networkService->openConnection(false);
```

### Connection Testing

```cpp
// Test full connection cycle
bool testResult = networkService->testConnection();

// Check current connection status
bool isConnected = networkService->isConnected();

// Check with fresh status (no cache)
bool freshStatus = networkService->isConnected(false);
```

### Connection Configuration

```cpp
// Set connection parameters
SDK::PaymentProcessor::SConnection connection;
connection.host = "api.example.com";
connection.port = 443;
connection.protocol = SDK::PaymentProcessor::ConnectionProtocol::HTTPS;

networkService->setConnection(connection);

// Get current connection parameters
auto currentConnection = networkService->getConnection();
```

## Network Task Manager

The Network Service provides access to the NetworkTaskManager for HTTP operations:

```cpp
NetworkTaskManager *taskManager = networkService->getNetworkTaskManager();

// Create and execute HTTP requests
// (See NetworkTaskManager documentation for details)
```

## Usage in Plugins

Network Service is commonly used in payment plugins for server communication:

```cpp
class PaymentPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mNetworkService = mCore->getNetworkService();
            mTaskManager = mNetworkService->getNetworkTaskManager();
        }

        return true;
    }

    void performPayment() {
        // Check connection before payment
        if (!mNetworkService->isConnected()) {
            LOG(mLog, LogLevel::Error, "No network connection for payment");
            return;
        }

        // Use task manager for payment request
        // ...
    }

private:
    INetworkService *mNetworkService;
    NetworkTaskManager *mTaskManager;
    ILog *mLog;
};
```

## Error Handling

```cpp
try {
    if (!networkService->openConnection()) {
        QString error = networkService->getLastConnectionError();
        LOG(log, LogLevel::Error, QString("Connection failed: %1").arg(error));
    }
} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Network service error: %1").arg(e.what()));
}
```

## Connection Monitoring

```cpp
// Periodic connection checking
QTimer *connectionTimer = new QTimer(this);
connect(connectionTimer, &QTimer::timeout, [this]() {
    if (!mNetworkService->isConnected()) {
        LOG(mLog, LogLevel::Warning, "Network connection lost");

        // Attempt reconnection
        if (mNetworkService->openConnection(false)) {
            LOG(mLog, LogLevel::Info, "Network connection restored");
        }
    }
});
connectionTimer->start(30000); // Check every 30 seconds
```

## Configuration

Network configuration is managed through the Settings Service:

```cpp
auto settings = core->getSettingsService();
auto adapter = settings->getAdapter("NetworkSettings");
// Configure timeouts, proxy settings, SSL options, etc.
```

## Thread Safety

Network operations are generally asynchronous and thread-safe. However, connection state changes should be handled carefully in multi-threaded environments.

## Dependencies

- Settings Service (for configuration)
- Event Service (for connection status notifications)
- Logging Service (for diagnostics)

## See Also

- [Crypt Service](crypt.md) - Secure communication
- [Settings Service](settings.md) - Network configuration
- [Event Service](event.md) - Connection status notifications
