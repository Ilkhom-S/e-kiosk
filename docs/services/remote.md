# Remote Service

The Remote Service manages command registration and status tracking for remote terminal management operations.

## Overview

The Remote Service (`IRemoteService`) provides:

- Remote command registration and queueing
- Command status tracking
- Payment operation handling
- Configuration and update management
- Query operations (screenshots, logs)

## Current Implementation - COMMAND QUEUE

The RemoteService provides a minimal interface for registering remote commands that are processed
by the monitoring server. It does NOT provide direct connection management, file transfer,
or real-time communication.

## Interface

```cpp
class IRemoteService : public QObject {
    Q_OBJECT

public:
    /// Command status
    enum EStatus {
        OK = 0,              /// Successfully executed
        Error,               /// Error
        Waiting,             /// Pending execution
        Executing,           /// Being executed
        Deleted,             /// Deleted
        PaymentNotFound      /// Payment not found
    };

    /// Update type
    enum EUpdateType {
        Configuration = 0,   /// Configuration update
        ServicePack,         /// Service pack (legacy)
        UserPack,            /// User content pack
        Update,              /// System update
        AdUpdate,            /// Advertisement content
        FirmwareDownload,    /// Firmware download
        FirmwareUpload,      /// Firmware installation
        CheckIntegrity       /// Integrity check
    };

    /// Payment command type
    enum EPaymentOperation {
        Process = 0,         /// Process payment
        Remove               /// Remove/cancel payment
    };

    /// Register terminal lock command
    virtual int registerLockCommand() = 0;

    /// Register terminal unlock command
    virtual int registerUnlockCommand() = 0;

    /// Register terminal reboot command
    virtual int registerRebootCommand() = 0;

    /// Register application restart command
    virtual int registerRestartCommand() = 0;

    /// Register terminal shutdown command
    virtual int registerShutdownCommand() = 0;

    /// Register payment operation command
    virtual int registerPaymentCommand(EPaymentOperation aOperation,
                                       const QString &aInitialSession,
                                       const QVariantMap &aParameters) = 0;

    /// Register update command
    virtual int registerUpdateCommand(EUpdateType aType,
                                      const QUrl &aConfigUrl,
                                      const QUrl &aUpdateUrl,
                                      const QString &aComponents) = 0;

    /// Register screenshot capture command
    virtual int registerScreenshotCommand() = 0;

    /// Register key generation command
    virtual int registerGenerateKeyCommand(const QString &login,
                                          const QString &password) = 0;

    /// Register arbitrary command
    virtual int registerAnyCommand() = 0;

    /// Mark command status as sent
    virtual void commandStatusSent(int aCommandId, int aStatus) = 0;
};
```

## Command Registration

### Basic Command Registration

```cpp
// Get remote service from core
auto remoteService = core->getRemoteService();

if (!remoteService) {
    LOG(log, LogLevel::Error, "Remote service not available");
   return;
}

// Register lock command
int commandId = remoteService->registerLockCommand();
LOG(log, LogLevel::Info, QString("Lock command registered with ID: %1").arg(commandId));
```

### Terminal Control Commands

```cpp
// Lock terminal
int lockId = remoteService->registerLockCommand();

// Unlock terminal
int unlockId = remoteService->registerUnlockCommand();

// Reboot system
int rebootId = remoteService->registerRebootCommand();

// Restart application
int restartId = remoteService->registerRestartCommand();

// Shutdown terminal
int shutdownId = remoteService->registerShutdownCommand();
```

### Query Commands

```cpp
// Request screenshot
int screenshotId = remoteService->registerScreenshotCommand();

// Request key generation
int keyId = remoteService->registerGenerateKeyCommand("admin", "password");
```

### Update Commands

```cpp
// Register configuration update
int configId = remoteService->registerUpdateCommand(
    IRemoteService::Configuration,
    QUrl("https://server.com/config"),
    QUrl("https://server.com/config/file"),
    "system"
);

// Register full system update
int updateId = remoteService->registerUpdateCommand(
    IRemoteService::Update,
    QUrl("https://server.com/meta"),
    QUrl("https://server.com/update.tar.gz"),
    "all"
);

// Register advertisement content update  
int adUpdateId = remoteService->registerUpdateCommand(
    IRemoteService::AdUpdate,
    QUrl("https://server.com/ad/meta"),
    QUrl("https://server.com/ad/content"),
    "media"
);
```

### Payment Operations

```cpp
// Process payment (modify transaction)
QVariantMap paymentParams = {
    {"amount", 5000},
    {"currency", "RUB"},
    {"approved", true}
};

int paymentId = remoteService->registerPaymentCommand(
    IRemoteService::Process,
    "session_12345",
    paymentParams
);

// Remove/cancel payment
int removeId = remoteService->registerPaymentCommand(
    IRemoteService::Remove,
    "session_12345",
    QVariantMap()
);
```

## Command Status Reporting

```cpp
// Execute command and report status
void handleRemoteCommand(int commandId) {
    try {
        // Perform command operation
        bool success = executeRemoteCommand(commandId);
        
        int statusCode = success ? IRemoteService::OK : IRemoteService::Error;
        remoteService->commandStatusSent(commandId, statusCode);
        
        LOG(log, LogLevel::Info, QString("Command %1 status sent: %2")
            .arg(commandId).arg(statusCode));
            
    } catch (const std::exception& e) {
        remoteService->commandStatusSent(commandId, IRemoteService::Error);
        LOG(log, LogLevel::Error, QString("Command %1 failed: %2")
            .arg(commandId).arg(e.what()));
    }
}
```

## Limitations

- **No direct remote access**: Commands are queued; monitoring server manages execution
- **No file transfer**: Use separate methods for file operations
- **No real-time connection**: Asynchronous command-based system only
- **No configuration retrieval**: Monitoring server handles remote configuration
- **No telemetry streaming**: Status reporting only, not real-time metrics
- **No remote shell**: No arbitrary command execution beyond predefined types

## Command Lifecycle

1. Plugin calls `registerXxxCommand()` → returns command ID
2. Command queued for monitoring server transmission
3. Monitoring server receives and processes command
4. Terminal executes command
5. Plugin reports status via `commandStatusSent(id, status)`
6. Monitoring server receives status confirmation

## Error Handling

```cpp
// Handle command registration failure
int commandId = remoteService->registerLockCommand();

if (commandId <= 0) {
    LOG(log, LogLevel::Error, "Failed to register lock command");
    return;
}

// Handle command execution error
try {
    if (!executeCommand(commandId)) {
        remoteService->commandStatusSent(commandId, IRemoteService::Error);
    }
} catch (const std::exception& e) {
    remoteService->commandStatusSent(commandId, IRemoteService::Error);
}
```

## Integration with Other Services

- **Terminal Service**: Controls terminal state (lock/unlock)
- **Scheduler Service**: May schedule remote operations
- **Settings Service**: Manages monitoring server configuration

## Related Components

- [Terminal Service](terminal.md) - Terminal control operations
- [Scheduler Service](scheduler.md) - Scheduled operations
- [Settings Service](settings.md) - Configuration management

## File Reference


- Implementation: [IRemoteService.h](../../include/SDK/PaymentProcessor/Core/IRemoteService.h)
