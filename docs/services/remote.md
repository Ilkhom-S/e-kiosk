# Remote Service

The Remote Service manages remote communication, management, and monitoring capabilities for the EKiosk system.

## Overview

The Remote Service (`IRemoteService`) handles:

- Remote terminal management and monitoring
- Command execution and response handling
- File transfer and synchronization
- Remote configuration updates
- Status reporting and telemetry
- Remote diagnostics and troubleshooting
- Secure remote access protocols

## Interface

```cpp
class IRemoteService : public QObject {
    Q_OBJECT

public:
    enum ConnectionState { Disconnected, Connecting, Connected, Error };
    enum CommandType { Status, Config, Update, Restart, Shutdown, Custom };

    struct RemoteCommand {
        QString id;
        CommandType type;
        QVariantMap parameters;
        QDateTime timestamp;
        QString source;      // Command source identifier
    };

    struct CommandResponse {
        QString commandId;
        bool success;
        QVariantMap result;
        QString errorMessage;
        QDateTime timestamp;
    };

    /// Connect to remote management server
    virtual bool connectToServer(const QString &serverUrl, const QString &authToken) = 0;

    /// Disconnect from remote server
    virtual void disconnectFromServer() = 0;

    /// Get connection state
    virtual ConnectionState getConnectionState() const = 0;

    /// Send status update to server
    virtual bool sendStatusUpdate(const QVariantMap &status) = 0;

    /// Execute remote command
    virtual CommandResponse executeCommand(const RemoteCommand &command) = 0;

    /// Send file to remote server
    virtual bool sendFile(const QString &localPath, const QString &remotePath) = 0;

    /// Receive file from remote server
    virtual bool receiveFile(const QString &remotePath, const QString &localPath) = 0;

    /// Update configuration from remote
    virtual bool updateConfiguration(const QVariantMap &config) = 0;

    /// Get remote server information
    virtual QVariantMap getServerInfo() const = 0;

    // ... additional methods for remote management
};
```

## Connection Management

### Connecting to Remote Server

```cpp
// Get remote service from core
auto remoteService = core->getRemoteService();

if (!remoteService) {
    LOG(log, LogLevel::Error, "Remote service not available");
    return;
}

// Connect to remote management server
QString serverUrl = "https://management.ekiosk.com/api/v1";
QString authToken = getAuthToken();  // From secure storage

bool connected = remoteService->connectToServer(serverUrl, authToken);

if (connected) {
    LOG(log, LogLevel::Info, "Connected to remote management server");
} else {
    LOG(log, LogLevel::Error, "Failed to connect to remote server");
}
```

### Connection Monitoring

```cpp
void monitorConnection() {
    IRemoteService::ConnectionState state = remoteService->getConnectionState();

    switch (state) {
        case IRemoteService::Disconnected:
            LOG(log, LogLevel::Warning, "Remote connection lost");
            handleDisconnectedState();
            break;

        case IRemoteService::Connecting:
            LOG(log, LogLevel::Info, "Connecting to remote server...");
            break;

        case IRemoteService::Connected:
            LOG(log, LogLevel::Info, "Remote connection active");
            break;

        case IRemoteService::Error:
            LOG(log, LogLevel::Error, "Remote connection error");
            handleConnectionError();
            break;
    }
}

void handleDisconnectedState() {
    // Attempt reconnection
    if (shouldAttemptReconnection()) {
        attemptReconnection();
    } else {
        // Queue data for later transmission
        queueOfflineData();
    }
}
```

## Status Reporting

### Sending Status Updates

```cpp
void sendTerminalStatus() {
    QVariantMap status = {
        {"timestamp", QDateTime::currentDateTime()},
        {"terminalId", getTerminalId()},
        {"location", getTerminalLocation()},
        {"version", getSoftwareVersion()},
        {"uptime", getSystemUptime()},
        {"transactionCount", getTransactionCount()},
        {"errorCount", getErrorCount()},
        {"memoryUsage", getMemoryUsage()},
        {"diskUsage", getDiskUsage()},
        {"networkStatus", getNetworkStatus()},
        {"lastTransaction", getLastTransactionTime()}
    };

    bool sent = remoteService->sendStatusUpdate(status);

    if (sent) {
        LOG(log, LogLevel::Info, "Status update sent to remote server");
    } else {
        LOG(log, LogLevel::Warning, "Failed to send status update - queuing for retry");
        queueStatusForRetry(status);
    }
}

void sendPeriodicStatusReports() {
    // Send status every 5 minutes
    QTimer *statusTimer = new QTimer(this);
    connect(statusTimer, &QTimer::timeout, this, &RemoteManagementPlugin::sendTerminalStatus);
    statusTimer->start(300000); // 5 minutes
}
```

### Telemetry Data

```cpp
void sendTelemetryData() {
    QVariantMap telemetry = {
        {"timestamp", QDateTime::currentDateTime()},
        {"terminalId", getTerminalId()},
        {"performance", collectPerformanceMetrics()},
        {"usage", collectUsageStatistics()},
        {"errors", collectErrorLogs()},
        {"transactions", collectTransactionData()}
    };

    bool sent = remoteService->sendStatusUpdate(telemetry);

    if (sent) {
        LOG(log, LogLevel::Info, "Telemetry data sent");
    } else {
        LOG(log, LogLevel::Warning, "Failed to send telemetry data");
    }
}

QVariantMap collectPerformanceMetrics() {
    return QVariantMap{
        {"cpuUsage", getCpuUsage()},
        {"memoryUsage", getMemoryUsage()},
        {"diskUsage", getDiskUsage()},
        {"networkLatency", getNetworkLatency()},
        {"responseTime", getAverageResponseTime()}
    };
}
```

## Remote Command Execution

### Handling Remote Commands

```cpp
void processRemoteCommand(const IRemoteService::RemoteCommand &command) {
    LOG(log, LogLevel::Info, QString("Processing remote command: %1 from %2")
        .arg(command.id, command.source));

    IRemoteService::CommandResponse response = remoteService->executeCommand(command);

    if (response.success) {
        LOG(log, LogLevel::Info, QString("Remote command executed successfully: %1").arg(command.id));

        // Handle successful execution
        handleCommandSuccess(response);

    } else {
        LOG(log, LogLevel::Error, QString("Remote command failed: %1 - %2")
            .arg(command.id, response.errorMessage));

        // Handle command failure
        handleCommandFailure(response);
    }
}

IRemoteService::CommandResponse executeStatusCommand(const IRemoteService::RemoteCommand &command) {
    IRemoteService::CommandResponse response;
    response.commandId = command.id;
    response.timestamp = QDateTime::currentDateTime();

    try {
        // Collect current status
        QVariantMap status = collectCurrentStatus();

        response.success = true;
        response.result = status;

    } catch (const std::exception &e) {
        response.success = false;
        response.errorMessage = e.what();
    }

    return response;
}

IRemoteService::CommandResponse executeConfigUpdateCommand(const IRemoteService::RemoteCommand &command) {
    IRemoteService::CommandResponse response;
    response.commandId = command.id;
    response.timestamp = QDateTime::currentDateTime();

    try {
        QVariantMap newConfig = command.parameters.value("config").toMap();

        // Validate configuration
        if (!validateConfiguration(newConfig)) {
            throw std::runtime_error("Invalid configuration provided");
        }

        // Apply configuration
        bool updated = applyConfiguration(newConfig);

        if (!updated) {
            throw std::runtime_error("Failed to apply configuration");
        }

        response.success = true;
        response.result = {{"message", "Configuration updated successfully"}};

    } catch (const std::exception &e) {
        response.success = false;
        response.errorMessage = e.what();
    }

    return response;
}
```

### Command Routing

```cpp
void setupCommandHandlers() {
    // Register command handlers
    m_commandHandlers[IRemoteService::Status] = [this](const IRemoteService::RemoteCommand &cmd) {
        return executeStatusCommand(cmd);
    };

    m_commandHandlers[IRemoteService::Config] = [this](const IRemoteService::RemoteCommand &cmd) {
        return executeConfigUpdateCommand(cmd);
    };

    m_commandHandlers[IRemoteService::Update] = [this](const IRemoteService::RemoteCommand &cmd) {
        return executeSoftwareUpdateCommand(cmd);
    };

    m_commandHandlers[IRemoteService::Restart] = [this](const IRemoteService::RemoteCommand &cmd) {
        return executeRestartCommand(cmd);
    };

    m_commandHandlers[IRemoteService::Shutdown] = [this](const IRemoteService::RemoteCommand &cmd) {
        return executeShutdownCommand(cmd);
    };
}

IRemoteService::CommandResponse routeCommand(const IRemoteService::RemoteCommand &command) {
    auto handler = m_commandHandlers.value(command.type);

    if (handler) {
        return handler(command);
    } else {
        IRemoteService::CommandResponse response;
        response.commandId = command.id;
        response.success = false;
        response.errorMessage = QString("Unknown command type: %1").arg(static_cast<int>(command.type));
        response.timestamp = QDateTime::currentDateTime();
        return response;
    }
}
```

## File Transfer

### Sending Files

```cpp
void sendLogFiles() {
    QStringList logFiles = getLogFilesToSend();

    foreach (const QString &localPath, logFiles) {
        QString remotePath = QString("logs/%1/%2")
                            .arg(getTerminalId(), QFileInfo(localPath).fileName());

        bool sent = remoteService->sendFile(localPath, remotePath);

        if (sent) {
            LOG(log, LogLevel::Info, QString("Log file sent: %1").arg(localPath));

            // Mark file as sent
            markFileAsSent(localPath);

        } else {
            LOG(log, LogLevel::Error, QString("Failed to send log file: %1").arg(localPath));
        }
    }
}

void sendScreenshot() {
    // Capture screenshot
    QString screenshotPath = captureScreenshot();

    if (screenshotPath.isEmpty()) {
        LOG(log, LogLevel::Error, "Failed to capture screenshot");
        return;
    }

    QString remotePath = QString("screenshots/%1_%2.png")
                        .arg(getTerminalId(), QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

    bool sent = remoteService->sendFile(screenshotPath, remotePath);

    if (sent) {
        LOG(log, LogLevel::Info, "Screenshot sent to remote server");

        // Clean up local file
        QFile::remove(screenshotPath);

    } else {
        LOG(log, LogLevel::Error, "Failed to send screenshot");
    }
}
```

### Receiving Files

```cpp
void checkForRemoteFiles() {
    // Check for configuration updates
    QString remoteConfigPath = QString("config/%1/config.json").arg(getTerminalId());
    QString localConfigPath = getLocalConfigPath();

    bool received = remoteService->receiveFile(remoteConfigPath, localConfigPath);

    if (received) {
        LOG(log, LogLevel::Info, "Configuration file received from remote server");

        // Apply new configuration
        applyRemoteConfiguration(localConfigPath);

    } else {
        LOG(log, LogLevel::Debug, "No new configuration file available");
    }

    // Check for software updates
    checkForSoftwareUpdates();
}

void downloadSoftwareUpdate(const QString &updateVersion) {
    QString remoteUpdatePath = QString("updates/%1/ekiosk_update_%1.zip").arg(updateVersion);
    QString localUpdatePath = QString("%1/ekiosk_update_%1.zip").arg(getTempDirectory(), updateVersion);

    bool received = remoteService->receiveFile(remoteUpdatePath, localUpdatePath);

    if (received) {
        LOG(log, LogLevel::Info, QString("Software update downloaded: %1").arg(updateVersion));

        // Install update
        installSoftwareUpdate(localUpdatePath);

    } else {
        LOG(log, LogLevel::Error, QString("Failed to download software update: %1").arg(updateVersion));
    }
}
```

## Configuration Management

### Remote Configuration Updates

```cpp
void applyRemoteConfiguration(const QString &configPath) {
    try {
        // Load configuration from file
        QVariantMap newConfig = loadConfigFromFile(configPath);

        // Validate configuration
        if (!validateConfiguration(newConfig)) {
            throw std::runtime_error("Invalid remote configuration");
        }

        // Apply configuration through remote service
        bool updated = remoteService->updateConfiguration(newConfig);

        if (updated) {
            LOG(log, LogLevel::Info, "Remote configuration applied successfully");

            // Notify other components of configuration change
            notifyConfigurationChange(newConfig);

        } else {
            throw std::runtime_error("Failed to apply remote configuration");
        }

    } catch (const std::exception &e) {
        LOG(log, LogLevel::Error, QString("Remote configuration error: %1").arg(e.what()));

        // Revert to previous configuration if needed
        revertConfiguration();
    }
}

void notifyConfigurationChange(const QVariantMap &newConfig) {
    // Publish configuration change event
    QVariantMap eventData = {
        {"type", "config_changed"},
        {"config", newConfig},
        {"timestamp", QDateTime::currentDateTime()}
    };

    // Emit signal or publish to event service
    emit configurationChanged(eventData);
}
```

## Usage in Plugins

Remote Service is commonly used in management and monitoring plugins:

```cpp
class RemoteManagementPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mRemoteService = mCore->getRemoteService();
            mEventService = mCore->getEventService();
            mSettingsService = mCore->getSettingsService();
            mLog = kernel->getLog("RemoteManagement");
        }

        return true;
    }

    bool start() override {
        // Initialize remote management
        initializeRemoteManagement();
        return true;
    }

    void initializeRemoteManagement() {
        // Load remote server configuration
        loadRemoteConfig();

        // Connect to remote server
        connectToRemoteServer();

        // Set up command handlers
        setupCommandHandlers();

        // Start status reporting
        startStatusReporting();

        // Set up file transfer monitoring
        setupFileTransfer();
    }

    void loadRemoteConfig() {
        QString serverUrl = mSettingsService->getValue("remote/serverUrl").toString();
        QString authToken = mSettingsService->getValue("remote/authToken").toString();
        bool autoConnect = mSettingsService->getValue("remote/autoConnect", true).toBool();

        m_serverUrl = serverUrl;
        m_authToken = authToken;
        m_autoConnect = autoConnect;

        LOG(mLog, LogLevel::Info, "Remote configuration loaded");
    }

    void connectToRemoteServer() {
        if (!m_autoConnect) {
            LOG(mLog, LogLevel::Info, "Auto-connect disabled");
            return;
        }

        bool connected = mRemoteService->connectToServer(m_serverUrl, m_authToken);

        if (connected) {
            LOG(mLog, LogLevel::Info, "Connected to remote management server");

            // Send initial status
            sendInitialStatus();

        } else {
            LOG(mLog, LogLevel::Error, "Failed to connect to remote server");
            scheduleReconnection();
        }
    }

    void setupCommandHandlers() {
        // Connect to command reception signal
        connect(mRemoteService, &IRemoteService::commandReceived,
                this, &RemoteManagementPlugin::handleRemoteCommand);
    }

    void handleRemoteCommand(const IRemoteService::RemoteCommand &command) {
        LOG(mLog, LogLevel::Info, QString("Received remote command: %1").arg(command.id));

        // Route command to appropriate handler
        IRemoteService::CommandResponse response = routeCommand(command);

        // Send response back
        sendCommandResponse(response);
    }

    void startStatusReporting() {
        // Send initial status
        sendTerminalStatus();

        // Set up periodic status reporting
        m_statusTimer = new QTimer(this);
        connect(m_statusTimer, &QTimer::timeout, this, &RemoteManagementPlugin::sendTerminalStatus);

        int statusInterval = mSettingsService->getValue("remote/statusInterval", 300).toInt(); // 5 minutes
        m_statusTimer->start(statusInterval * 1000);
    }

    void sendTerminalStatus() {
        if (mRemoteService->getConnectionState() != IRemoteService::Connected) {
            LOG(mLog, LogLevel::Debug, "Not connected to remote server, skipping status update");
            return;
        }

        QVariantMap status = collectTerminalStatus();

        bool sent = mRemoteService->sendStatusUpdate(status);

        if (sent) {
            LOG(mLog, LogLevel::Info, "Status update sent to remote server");
        } else {
            LOG(mLog, LogLevel::Warning, "Failed to send status update");
            queueStatusForRetry(status);
        }
    }

    QVariantMap collectTerminalStatus() {
        return QVariantMap{
            {"timestamp", QDateTime::currentDateTime()},
            {"terminalId", getTerminalId()},
            {"location", getTerminalLocation()},
            {"version", getSoftwareVersion()},
            {"state", getTerminalState()},
            {"uptime", getSystemUptime()},
            {"transactionCount", getTransactionCount()},
            {"errorCount", getErrorCount()},
            {"memoryUsage", getMemoryUsage()},
            {"diskUsage", getDiskUsage()},
            {"networkStatus", getNetworkStatus()},
            {"lastTransaction", getLastTransactionTime()},
            {"activeConnections", getActiveConnectionCount()}
        };
    }

    void setupFileTransfer() {
        // Set up periodic file transfer checks
        m_fileTransferTimer = new QTimer(this);
        connect(m_fileTransferTimer, &QTimer::timeout, this, &RemoteManagementPlugin::checkForRemoteFiles);
        m_fileTransferTimer->start(3600000); // 1 hour

        // Send initial log files
        sendLogFiles();
    }

    void checkForRemoteFiles() {
        if (mRemoteService->getConnectionState() != IRemoteService::Connected) {
            return;
        }

        // Check for configuration updates
        QString remoteConfigPath = QString("config/%1/config.json").arg(getTerminalId());
        QString localConfigPath = getLocalConfigPath();

        bool received = mRemoteService->receiveFile(remoteConfigPath, localConfigPath);

        if (received) {
            LOG(mLog, LogLevel::Info, "Configuration update received");
            applyRemoteConfiguration(localConfigPath);
        }

        // Check for software updates
        checkForSoftwareUpdates();
    }

    void sendLogFiles() {
        if (mRemoteService->getConnectionState() != IRemoteService::Connected) {
            return;
        }

        QStringList logFiles = getPendingLogFiles();

        foreach (const QString &logFile, logFiles) {
            QString remotePath = QString("logs/%1/%2")
                                .arg(getTerminalId(), QFileInfo(logFile).fileName());

            bool sent = mRemoteService->sendFile(logFile, remotePath);

            if (sent) {
                LOG(mLog, LogLevel::Info, QString("Log file sent: %1").arg(logFile));
                markLogFileAsSent(logFile);
            } else {
                LOG(mLog, LogLevel::Warning, QString("Failed to send log file: %1").arg(logFile));
            }
        }
    }

    void handleConnectionStateChange(IRemoteService::ConnectionState newState) {
        switch (newState) {
            case IRemoteService::Connected:
                LOG(mLog, LogLevel::Info, "Remote connection established");
                onConnectionEstablished();
                break;

            case IRemoteService::Disconnected:
                LOG(mLog, LogLevel::Warning, "Remote connection lost");
                onConnectionLost();
                break;

            case IRemoteService::Error:
                LOG(mLog, LogLevel::Error, "Remote connection error");
                onConnectionError();
                break;

            default:
                break;
        }
    }

    void onConnectionEstablished() {
        // Send queued data
        sendQueuedData();

        // Send immediate status update
        sendTerminalStatus();

        // Reset reconnection attempts
        m_reconnectionAttempts = 0;
    }

    void onConnectionLost() {
        // Start reconnection process
        scheduleReconnection();
    }

    void onConnectionError() {
        LOG(mLog, LogLevel::Error, "Remote connection error occurred");

        // Increase reconnection delay
        m_reconnectionDelay *= 2;
        if (m_reconnectionDelay > 300000) { // Max 5 minutes
            m_reconnectionDelay = 300000;
        }

        scheduleReconnection();
    }

    void scheduleReconnection() {
        if (m_reconnectionTimer) {
            m_reconnectionTimer->stop();
        }

        m_reconnectionTimer = new QTimer(this);
        m_reconnectionTimer->setSingleShot(true);
        connect(m_reconnectionTimer, &QTimer::timeout, this, &RemoteManagementPlugin::attemptReconnection);
        m_reconnectionTimer->start(m_reconnectionDelay);
    }

    void attemptReconnection() {
        m_reconnectionAttempts++;

        LOG(mLog, LogLevel::Info, QString("Attempting reconnection (%1)").arg(m_reconnectionAttempts));

        bool connected = mRemoteService->connectToServer(m_serverUrl, m_authToken);

        if (!connected) {
            if (m_reconnectionAttempts < 10) { // Max 10 attempts
                scheduleReconnection();
            } else {
                LOG(mLog, LogLevel::Error, "Maximum reconnection attempts reached");
            }
        }
    }

    void sendCommandResponse(const IRemoteService::CommandResponse &response) {
        // Implementation depends on remote service interface
        // This would send the response back to the remote server
        QVariantMap responseData = {
            {"commandId", response.commandId},
            {"success", response.success},
            {"result", response.result},
            {"errorMessage", response.errorMessage},
            {"timestamp", response.timestamp}
        };

        // Send response via remote service
        mRemoteService->sendStatusUpdate({{"commandResponse", responseData}});
    }

    void queueStatusForRetry(const QVariantMap &status) {
        m_queuedStatuses.append(status);

        // Limit queue size
        if (m_queuedStatuses.size() > 50) {
            m_queuedStatuses.removeFirst();
        }
    }

    void sendQueuedData() {
        // Send queued status updates
        foreach (const QVariantMap &status, m_queuedStatuses) {
            bool sent = mRemoteService->sendStatusUpdate(status);
            if (sent) {
                LOG(mLog, LogLevel::Info, "Queued status update sent");
            }
        }

        m_queuedStatuses.clear();
    }

private:
    IRemoteService *mRemoteService;
    IEventService *mEventService;
    ISettingsService *mSettingsService;
    ILog *mLog;

    QString m_serverUrl;
    QString m_authToken;
    bool m_autoConnect;

    QTimer *m_statusTimer;
    QTimer *m_fileTransferTimer;
    QTimer *m_reconnectionTimer;

    int m_reconnectionAttempts;
    int m_reconnectionDelay; // milliseconds

    QList<QVariantMap> m_queuedStatuses;
    QMap<IRemoteService::CommandType, std::function<IRemoteService::CommandResponse(const IRemoteService::RemoteCommand&)>> m_commandHandlers;
};
```

## Error Handling

```cpp
try {
    // Validate remote operation
    if (remoteService->getConnectionState() != IRemoteService::Connected) {
        throw std::runtime_error("Not connected to remote server");
    }

    // Check remote service availability
    if (!remoteService) {
        throw std::runtime_error("Remote service not available");
    }

    // Perform remote operation
    if (!remoteService->sendStatusUpdate(status)) {
        throw std::runtime_error("Failed to send status update");
    }

} catch (const std::runtime_error &e) {
    LOG(log, LogLevel::Error, QString("Remote service error: %1").arg(e.what()));

    // Handle error - queue for retry, attempt reconnection, etc.
    handleRemoteError(e.what());

} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Unexpected remote error: %1").arg(e.what()));
}
```

## Security Considerations

- Use secure communication protocols (HTTPS, WSS)
- Implement proper authentication and authorization
- Encrypt sensitive data in transit and at rest
- Validate all remote commands and file transfers
- Implement rate limiting for remote operations
- Audit all remote management activities

## Performance Considerations

- Implement connection pooling and keep-alive
- Use compression for large data transfers
- Implement retry logic with exponential backoff
- Queue operations during connectivity issues
- Monitor and limit resource usage for remote operations

## Dependencies

- Network Service (for connectivity)
- Settings Service (for remote configuration)
- Event Service (for remote event notifications)
- Security Service (for authentication)

## See Also

- [Network Service](../../modules/NetworkTaskManager/) - Connectivity
- [Settings Service](settings.md) - Remote configuration
- [Event Service](event.md) - Remote event notifications
- [Security Service](../../modules/Crypt/) - Authentication
