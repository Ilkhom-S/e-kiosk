# Terminal Service

The Terminal Service manages terminal-specific operations and kiosk mode functionality for the EKiosk system.

## Overview

The Terminal Service (`ITerminalService`) handles:

- Kiosk mode management and restrictions
- Terminal identification and configuration
- System lockdown and security
- Terminal status monitoring
- Remote management capabilities
- Hardware control and monitoring
- Terminal-specific settings

## Interface

```cpp
class ITerminalService : public QObject {
    Q_OBJECT

public:
    enum TerminalState { Active, Inactive, Maintenance, Error };
    enum KioskMode { Disabled, Basic, Strict, Custom };

    struct TerminalInfo {
        QString id;
        QString name;
        QString location;
        QString type;        // "kiosk", "self_service", "attended"
        TerminalState state;
        QDateTime lastActive;
        QVariantMap capabilities;
        QVariantMap configuration;
    };

    /// Get terminal information
    virtual TerminalInfo getTerminalInfo() const = 0;

    /// Set kiosk mode
    virtual bool setKioskMode(KioskMode mode) = 0;

    /// Get current kiosk mode
    virtual KioskMode getKioskMode() const = 0;

    /// Lock terminal (disable user input)
    virtual bool lockTerminal(const QString &reason = QString()) = 0;

    /// Unlock terminal
    virtual bool unlockTerminal() = 0;

    /// Check if terminal is locked
    virtual bool isLocked() const = 0;

    /// Restart terminal
    virtual bool restartTerminal() = 0;

    /// Shutdown terminal
    virtual bool shutdownTerminal() = 0;

    /// Send terminal status update
    virtual bool sendStatusUpdate(const QVariantMap &status) = 0;

    /// Get terminal configuration
    virtual QVariantMap getConfiguration() const = 0;

    /// Update terminal configuration
    virtual bool updateConfiguration(const QVariantMap &config) = 0;

    // ... additional methods for terminal management
};
```

## Terminal Information

### Getting Terminal Info

```cpp
// Get terminal service from core
auto terminalService = core->getTerminalService();

if (!terminalService) {
    LOG(log, LogLevel::Error, "Terminal service not available");
    return;
}

// Get terminal information
ITerminalService::TerminalInfo info = terminalService->getTerminalInfo();

LOG(log, LogLevel::Info, QString("Terminal ID: %1").arg(info.id));
LOG(log, LogLevel::Info, QString("Terminal Name: %1").arg(info.name));
LOG(log, LogLevel::Info, QString("Location: %1").arg(info.location));
LOG(log, LogLevel::Info, QString("Type: %1").arg(info.type));
LOG(log, LogLevel::Info, QString("State: %1").arg(static_cast<int>(info.state)));
LOG(log, LogLevel::Info, QString("Last Active: %1").arg(info.lastActive.toString()));
```

### Terminal Capabilities

```cpp
void checkTerminalCapabilities() {
    ITerminalService::TerminalInfo info = terminalService->getTerminalInfo();

    // Check hardware capabilities
    bool hasPrinter = info.capabilities.value("printer", false).toBool();
    bool hasCardReader = info.capabilities.value("card_reader", false).toBool();
    bool hasCashAcceptor = info.capabilities.value("cash_acceptor", false).toBool();
    bool hasTouchScreen = info.capabilities.value("touch_screen", false).toBool();

    LOG(log, LogLevel::Info, QString("Terminal capabilities - Printer: %1, Card Reader: %2, Cash: %3, Touch: %4")
        .arg(hasPrinter).arg(hasCardReader).arg(hasCashAcceptor).arg(hasTouchScreen));

    // Adjust functionality based on capabilities
    if (!hasTouchScreen) {
        enableKeyboardMode();
    }

    if (!hasPrinter) {
        disableReceiptPrinting();
    }
}
```

## Kiosk Mode Management

### Setting Kiosk Mode

```cpp
void configureKioskMode() {
    // Determine appropriate kiosk mode based on terminal type
    ITerminalService::TerminalInfo info = terminalService->getTerminalInfo();
    ITerminalService::KioskMode mode;

    if (info.type == "kiosk") {
        mode = ITerminalService::Strict;  // Maximum restrictions for public kiosk
    } else if (info.type == "self_service") {
        mode = ITerminalService::Basic;   // Basic restrictions for self-service
    } else {
        mode = ITerminalService::Disabled; // No restrictions for attended terminals
    }

    bool set = terminalService->setKioskMode(mode);

    if (set) {
        LOG(log, LogLevel::Info, QString("Kiosk mode set to: %1").arg(static_cast<int>(mode)));
    } else {
        LOG(log, LogLevel::Error, "Failed to set kiosk mode");
    }
}
```

### Kiosk Mode Restrictions

```cpp
void applyKioskRestrictions() {
    ITerminalService::KioskMode mode = terminalService->getKioskMode();

    switch (mode) {
        case ITerminalService::Disabled:
            // No restrictions
            enableFullSystemAccess();
            break;

        case ITerminalService::Basic:
            // Basic restrictions
            disableTaskManager();
            disableSystemShortcuts();
            hideDesktopIcons();
            break;

        case ITerminalService::Strict:
            // Maximum restrictions
            disableTaskManager();
            disableSystemShortcuts();
            hideDesktopIcons();
            disableAltTab();
            disableCtrlAltDel();
            lockToApplication();
            break;

        case ITerminalService::Custom:
            // Custom restrictions based on configuration
            applyCustomRestrictions();
            break;
    }

    LOG(log, LogLevel::Info, QString("Applied kiosk restrictions for mode: %1").arg(static_cast<int>(mode)));
}
```

## Terminal Lock Management

### Locking Terminal

```cpp
void lockTerminalForMaintenance() {
    QString reason = "Scheduled maintenance";

    bool locked = terminalService->lockTerminal(reason);

    if (locked) {
        LOG(log, LogLevel::Info, QString("Terminal locked for: %1").arg(reason));

        // Perform maintenance tasks
        performMaintenanceTasks();

        // Unlock after maintenance
        unlockTerminal();

    } else {
        LOG(log, LogLevel::Error, "Failed to lock terminal");
    }
}

void lockTerminalForSecurity(const QString &threatLevel) {
    QString reason = QString("Security lockdown - %1").arg(threatLevel);

    bool locked = terminalService->lockTerminal(reason);

    if (locked) {
        LOG(log, LogLevel::Warning, QString("Terminal locked for security: %1").arg(threatLevel));

        // Notify security system
        notifySecuritySystem(threatLevel);

        // Wait for security clearance
        waitForSecurityClearance();

    } else {
        LOG(log, LogLevel::Error, "Failed to lock terminal for security");
    }
}
```

### Terminal Status Checks

```cpp
void monitorTerminalStatus() {
    bool locked = terminalService->isLocked();
    ITerminalService::TerminalState state = terminalService->getTerminalInfo().state;

    if (locked) {
        LOG(log, LogLevel::Warning, "Terminal is currently locked");

        // Handle locked state
        handleLockedTerminal();

    } else {
        LOG(log, LogLevel::Info, "Terminal is unlocked");

        // Check operational state
        if (state == ITerminalService::Error) {
            LOG(log, LogLevel::Error, "Terminal is in error state");
            handleTerminalError();
        } else if (state == ITerminalService::Maintenance) {
            LOG(log, LogLevel::Info, "Terminal is in maintenance mode");
            handleMaintenanceMode();
        }
    }
}
```

## System Control

### Restart and Shutdown

```cpp
void handleSystemRestart() {
    // Check if restart is allowed
    if (!isRestartAllowed()) {
        LOG(log, LogLevel::Warning, "System restart not allowed at this time");
        return;
    }

    // Notify users of impending restart
    notifyUsersOfRestart();

    // Save current state
    saveSystemState();

    // Perform restart
    bool restarted = terminalService->restartTerminal();

    if (restarted) {
        LOG(log, LogLevel::Info, "Terminal restart initiated");
    } else {
        LOG(log, LogLevel::Error, "Failed to restart terminal");
    }
}

void handleSystemShutdown() {
    // Check shutdown conditions
    if (hasActiveTransactions()) {
        LOG(log, LogLevel::Warning, "Cannot shutdown - active transactions in progress");
        return;
    }

    // Complete pending operations
    completePendingOperations();

    // Notify remote management
    sendShutdownNotification();

    // Perform shutdown
    bool shutdown = terminalService->shutdownTerminal();

    if (shutdown) {
        LOG(log, LogLevel::Info, "Terminal shutdown initiated");
    } else {
        LOG(log, LogLevel::Error, "Failed to shutdown terminal");
    }
}
```

## Status Updates

### Sending Status Updates

```cpp
void sendTerminalStatus() {
    QVariantMap status = {
        {"timestamp", QDateTime::currentDateTime()},
        {"state", static_cast<int>(terminalService->getTerminalInfo().state)},
        {"locked", terminalService->isLocked()},
        {"kioskMode", static_cast<int>(terminalService->getKioskMode())},
        {"uptime", getSystemUptime()},
        {"memoryUsage", getMemoryUsage()},
        {"diskSpace", getAvailableDiskSpace()},
        {"networkStatus", getNetworkStatus()},
        {"lastTransaction", getLastTransactionTime()}
    };

    bool sent = terminalService->sendStatusUpdate(status);

    if (sent) {
        LOG(log, LogLevel::Info, "Terminal status update sent");
    } else {
        LOG(log, LogLevel::Warning, "Failed to send terminal status update");
    }
}

void sendErrorStatus(const QString &errorType, const QString &errorDetails) {
    QVariantMap errorStatus = {
        {"timestamp", QDateTime::currentDateTime()},
        {"type", "error"},
        {"errorType", errorType},
        {"errorDetails", errorDetails},
        {"terminalId", terminalService->getTerminalInfo().id},
        {"state", static_cast<int>(ITerminalService::Error)}
    };

    bool sent = terminalService->sendStatusUpdate(errorStatus);

    if (sent) {
        LOG(log, LogLevel::Error, QString("Error status sent: %1").arg(errorType));
    } else {
        LOG(log, LogLevel::Error, "Failed to send error status");
    }
}
```

## Configuration Management

### Getting Configuration

```cpp
void loadTerminalConfiguration() {
    QVariantMap config = terminalService->getConfiguration();

    // Load kiosk settings
    bool kioskEnabled = config.value("kiosk/enabled", false).toBool();
    QString kioskMode = config.value("kiosk/mode", "basic").toString();

    // Load security settings
    bool autoLockEnabled = config.value("security/autoLock", true).toBool();
    int lockTimeout = config.value("security/lockTimeout", 300).toInt(); // 5 minutes

    // Load maintenance settings
    QString maintenanceSchedule = config.value("maintenance/schedule", "daily").toString();
    QString maintenanceWindow = config.value("maintenance/window", "02:00-04:00").toString();

    // Apply configuration
    applyTerminalConfiguration(config);

    LOG(log, LogLevel::Info, "Terminal configuration loaded");
}
```

### Updating Configuration

```cpp
void updateTerminalSettings() {
    QVariantMap newConfig;

    // Update kiosk settings
    newConfig["kiosk/enabled"] = true;
    newConfig["kiosk/mode"] = "strict";

    // Update security settings
    newConfig["security/autoLock"] = true;
    newConfig["security/lockTimeout"] = 600; // 10 minutes

    // Update maintenance settings
    newConfig["maintenance/schedule"] = "weekly";
    newConfig["maintenance/window"] = "01:00-03:00";

    bool updated = terminalService->updateConfiguration(newConfig);

    if (updated) {
        LOG(log, LogLevel::Info, "Terminal configuration updated");

        // Reload configuration to apply changes
        loadTerminalConfiguration();

    } else {
        LOG(log, LogLevel::Error, "Failed to update terminal configuration");
    }
}
```

## Usage in Plugins

Terminal Service is commonly used in system management and security plugins:

```cpp
class TerminalManagementPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mTerminalService = mCore->getTerminalService();
            mEventService = mCore->getEventService();
            mSettingsService = mCore->getSettingsService();
            mLog = kernel->getLog("TerminalManagement");
        }

        return true;
    }

    bool start() override {
        // Initialize terminal management
        initializeTerminalManagement();
        return true;
    }

    void initializeTerminalManagement() {
        // Get terminal information
        ITerminalService::TerminalInfo info = mTerminalService->getTerminalInfo();

        LOG(mLog, LogLevel::Info, QString("Managing terminal: %1 (%2)").arg(info.name, info.id));

        // Configure kiosk mode
        configureKioskMode();

        // Set up status monitoring
        setupStatusMonitoring();

        // Initialize security features
        initializeSecurityFeatures();

        // Load terminal configuration
        loadTerminalConfig();
    }

    void configureKioskMode() {
        // Determine kiosk mode based on terminal type and settings
        QString terminalType = mTerminalService->getTerminalInfo().type;
        bool kioskEnabled = mSettingsService->getValue("terminal/kiosk/enabled", true).toBool();

        ITerminalService::KioskMode mode = ITerminalService::Disabled;

        if (kioskEnabled) {
            if (terminalType == "kiosk") {
                mode = ITerminalService::Strict;
            } else if (terminalType == "self_service") {
                mode = ITerminalService::Basic;
            }
        }

        bool set = mTerminalService->setKioskMode(mode);

        if (set) {
            LOG(mLog, LogLevel::Info, QString("Kiosk mode configured: %1").arg(static_cast<int>(mode)));
        } else {
            LOG(mLog, LogLevel::Error, "Failed to configure kiosk mode");
        }
    }

    void setupStatusMonitoring() {
        // Set up periodic status reporting
        m_statusTimer = new QTimer(this);
        connect(m_statusTimer, &QTimer::timeout, this, &TerminalManagementPlugin::sendStatusReport);
        m_statusTimer->start(300000); // 5 minutes

        // Subscribe to terminal events
        mEventService->subscribe("terminal.status_request", [this](const QVariantMap &eventData) {
            sendImmediateStatusReport();
        });

        mEventService->subscribe("terminal.lock_request", [this](const QVariantMap &eventData) {
            QString reason = eventData.value("reason", "Remote lock").toString();
            handleRemoteLockRequest(reason);
        });

        LOG(mLog, LogLevel::Info, "Status monitoring configured");
    }

    void sendStatusReport() {
        QVariantMap status = collectTerminalStatus();

        bool sent = mTerminalService->sendStatusUpdate(status);

        if (sent) {
            LOG(mLog, LogLevel::Info, "Status report sent");
        } else {
            LOG(mLog, LogLevel::Warning, "Failed to send status report");
        }
    }

    void sendImmediateStatusReport() {
        QVariantMap status = collectTerminalStatus();
        status["immediate"] = true;

        bool sent = mTerminalService->sendStatusUpdate(status);

        if (sent) {
            LOG(mLog, LogLevel::Info, "Immediate status report sent");
        }
    }

    QVariantMap collectTerminalStatus() {
        ITerminalService::TerminalInfo info = mTerminalService->getTerminalInfo();

        return QVariantMap{
            {"timestamp", QDateTime::currentDateTime()},
            {"terminalId", info.id},
            {"terminalName", info.name},
            {"location", info.location},
            {"state", static_cast<int>(info.state)},
            {"locked", mTerminalService->isLocked()},
            {"kioskMode", static_cast<int>(mTerminalService->getKioskMode())},
            {"uptime", getSystemUptime()},
            {"cpuUsage", getCpuUsage()},
            {"memoryUsage", getMemoryUsage()},
            {"diskUsage", getDiskUsage()},
            {"networkConnected", isNetworkConnected()},
            {"lastTransaction", getLastTransactionTime()},
            {"activeUsers", getActiveUserCount()},
            {"errorCount", getErrorCount()},
            {"transactionCount", getTransactionCount()}
        };
    }

    void initializeSecurityFeatures() {
        // Set up auto-lock timer
        int lockTimeout = mSettingsService->getValue("terminal/security/lockTimeout", 300).toInt();

        m_autoLockTimer = new QTimer(this);
        m_autoLockTimer->setSingleShot(true);
        connect(m_autoLockTimer, &QTimer::timeout, [this]() {
            autoLockTerminal();
        });

        // Connect to user activity signals
        connectToUserActivitySignals();

        LOG(mLog, LogLevel::Info, "Security features initialized");
    }

    void handleUserActivity() {
        // Reset auto-lock timer on user activity
        int lockTimeout = mSettingsService->getValue("terminal/security/lockTimeout", 300).toInt();
        m_autoLockTimer->start(lockTimeout * 1000);
    }

    void autoLockTerminal() {
        if (!mTerminalService->isLocked()) {
            bool locked = mTerminalService->lockTerminal("Auto-lock due to inactivity");

            if (locked) {
                LOG(mLog, LogLevel::Info, "Terminal auto-locked due to inactivity");

                // Publish lock event
                mEventService->publish("terminal.auto_locked", {
                    {"reason", "inactivity"},
                    {"timestamp", QDateTime::currentDateTime()}
                });
            }
        }
    }

    void handleRemoteLockRequest(const QString &reason) {
        bool locked = mTerminalService->lockTerminal(reason);

        if (locked) {
            LOG(mLog, LogLevel::Warning, QString("Terminal locked remotely: %1").arg(reason));

            // Publish lock event
            mEventService->publish("terminal.remote_locked", {
                {"reason", reason},
                {"timestamp", QDateTime::currentDateTime()}
            });

        } else {
            LOG(mLog, LogLevel::Error, "Failed to lock terminal remotely");
        }
    }

    void loadTerminalConfig() {
        QVariantMap config = mTerminalService->getConfiguration();

        // Apply configuration settings
        applyConfigurationSettings(config);

        LOG(mLog, LogLevel::Info, "Terminal configuration loaded and applied");
    }

    void handleMaintenanceMode() {
        // Check if maintenance is scheduled
        if (isMaintenanceWindow()) {
            // Enter maintenance mode
            enterMaintenanceMode();
        }
    }

    void enterMaintenanceMode() {
        // Set terminal state to maintenance
        QVariantMap status = {
            {"state", static_cast<int>(ITerminalService::Maintenance)},
            {"maintenanceMode", true},
            {"timestamp", QDateTime::currentDateTime()}
        };

        mTerminalService->sendStatusUpdate(status);

        // Perform maintenance tasks
        performMaintenanceTasks();

        LOG(mLog, LogLevel::Info, "Entered maintenance mode");
    }

    void performMaintenanceTasks() {
        // Clean up temporary files
        cleanupTempFiles();

        // Update software if needed
        checkForUpdates();

        // Run diagnostics
        runSystemDiagnostics();

        // Send maintenance report
        sendMaintenanceReport();
    }

    void handleTerminalError() {
        // Collect error information
        QVariantMap errorInfo = {
            {"errorType", "terminal_error"},
            {"errorDetails", getLastErrorDetails()},
            {"timestamp", QDateTime::currentDateTime()},
            {"terminalId", mTerminalService->getTerminalInfo().id}
        };

        // Send error status
        mTerminalService->sendStatusUpdate(errorInfo);

        // Attempt error recovery
        attemptErrorRecovery();

        LOG(mLog, LogLevel::Error, "Terminal error handled");
    }

    void attemptErrorRecovery() {
        // Try to recover from error
        if (canRecoverFromError()) {
            bool recovered = performErrorRecovery();

            if (recovered) {
                LOG(mLog, LogLevel::Info, "Error recovery successful");
            } else {
                LOG(mLog, LogLevel::Warning, "Error recovery failed");
            }
        } else {
            // Schedule restart if recovery not possible
            scheduleTerminalRestart();
        }
    }

private:
    ITerminalService *mTerminalService;
    IEventService *mEventService;
    ISettingsService *mSettingsService;
    ILog *mLog;

    QTimer *m_statusTimer;
    QTimer *m_autoLockTimer;
};
```

## Error Handling

```cpp
try {
    // Validate terminal operation
    if (terminalService->isLocked()) {
        throw std::runtime_error("Terminal is currently locked");
    }

    // Check terminal service availability
    if (!terminalService) {
        throw std::runtime_error("Terminal service not available");
    }

    // Perform terminal operation
    if (!terminalService->restartTerminal()) {
        throw std::runtime_error("Failed to restart terminal");
    }

} catch (const std::runtime_error &e) {
    LOG(log, LogLevel::Error, QString("Terminal service error: %1").arg(e.what()));

    // Handle error - notify admin, attempt recovery, etc.
    handleTerminalError(e.what());

} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Unexpected terminal error: %1").arg(e.what()));
}
```

## Remote Management

```cpp
void handleRemoteCommand(const QVariantMap &command) {
    QString commandType = command.value("type").toString();

    try {
        if (commandType == "lock") {
            QString reason = command.value("reason", "Remote command").toString();
            terminalService->lockTerminal(reason);

        } else if (commandType == "unlock") {
            terminalService->unlockTerminal();

        } else if (commandType == "restart") {
            terminalService->restartTerminal();

        } else if (commandType == "shutdown") {
            terminalService->shutdownTerminal();

        } else if (commandType == "update_config") {
            QVariantMap newConfig = command.value("config").toMap();
            terminalService->updateConfiguration(newConfig);

        } else {
            throw std::invalid_argument(QString("Unknown command type: %1").arg(commandType).toString());
        }

        LOG(log, LogLevel::Info, QString("Remote command executed: %1").arg(commandType));

    } catch (const std::exception &e) {
        LOG(log, LogLevel::Error, QString("Remote command failed: %1").arg(e.what()));
    }
}
```

## Security Considerations

- Implement secure remote management protocols
- Validate all remote commands and parameters
- Audit all terminal state changes
- Secure terminal configuration storage
- Implement access controls for management operations

## Performance Considerations

- Minimize status update frequency during high load
- Use efficient state monitoring
- Cache terminal configuration
- Implement background maintenance operations

## Dependencies

- Settings Service (for terminal configuration)
- Event Service (for terminal event notifications)
- Network Service (for remote management)
- Security Service (for access control)

## See Also

- [Settings Service](settings.md) - Terminal configuration
- [Event Service](event.md) - Terminal event notifications
- [Network Service](../../modules/NetworkTaskManager/) - Remote management
- [Security Service](../../modules/Crypt/) - Access control
