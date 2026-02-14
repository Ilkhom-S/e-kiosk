# Terminal Service

The Terminal Service manages terminal state, locking, and error handling for the EKiosk system.

## Overview

The Terminal Service (`ITerminalService`) manages:

- Terminal lock/unlock state
- Terminal error conditions
- Terminal status and configuration
- Configuration update requests

## Current Implementation - BASIC INTERFACE

The TerminalService provides minimal functionality focused on lock state and error handling.
Most documented features (kiosk modes, terminal info structs, hardware control) are NOT implemented.

## Interface

```cpp
class ITerminalService {
public:
    /// Check if terminal is locked
    virtual bool isLocked() const = 0;

    /// Set terminal lock state
    virtual void setLock(bool aIsLocked) = 0;

    /// Request configuration update
    virtual void needUpdateConfigs() = 0;

    /// Set terminal error condition
    virtual void setTerminalError(ETerminalError::Enum aErrorType,
                                  const QString &aDescription) = 0;

    /// Check if terminal has specific error
    virtual bool isTerminalError(ETerminalError::Enum aErrorType) const = 0;

    /// Send feedback from subsystem
    virtual void sendFeedback(const QString &aSenderSubsystem,
                             const QString &aMessage) = 0;
};
```

## Lock Management

### Locking Terminal

```cpp
// Get terminal service from core
auto terminalService = core->getTerminalService();

if (!terminalService) {
    LOG(log, LogLevel::Error, "Terminal service not available");
    return;
}

// Lock terminal (disable user input)
terminalService->setLock(true);
LOG(log, LogLevel::Info, "Terminal locked");
```

### Unlocking Terminal

```cpp
// Unlock terminal (allow user input)
terminalService->setLock(false);
LOG(log, LogLevel::Info, "Terminal unlocked");
```

### Checking Lock State

```cpp
// Check if terminal is locked
if (terminalService->isLocked()) {
    LOG(log, LogLevel::Info, "Terminal is currently locked");
} else {
    LOG(log, LogLevel::Info, "Terminal is unlocked");
}
```

## Error Management

### Setting Terminal Error

```cpp
// Set specific terminal error
terminalService->setTerminalError(
    ETerminalError::DeviceError,
    "Payment device not responding"
);
```

### Checking for Error

```cpp
// Check if terminal has specific error condition
if (terminalService->isTerminalError(ETerminalError::NetworkError)) {
    LOG(log, LogLevel::Warning, "Terminal has network error");
}
```

## Configuration Management

### Request Configuration Update

```cpp
// Request that terminal reload configuration files
terminalService->needUpdateConfigs();
LOG(log, LogLevel::Info, "Configuration update requested");
```

## Feedback System

### Send System Feedback

```cpp
// Send feedback message from subsystem
terminalService->sendFeedback(
    "PaymentService",
    "Payment processing delay detected"
);
```

## Error Types

The `ETerminalError::Enum` defines terminal error conditions:

- `DeviceError` - Hardware device malfunction
- `NetworkError` - Network connectivity issue
- `ConfigError` - Configuration problem
- `ServiceError` - Service runtime error
- (See HookConstants.h for complete list)

## Limitations

- **No kiosk mode management**: Kiosk mode features are not implemented
- **No terminal info retrieval**: Cannot get terminal ID, name, or capabilities
- **No hardware control**: Hardware management not available through this service
- **No terminal state enum**: Terminal state management not available
- **Basic lock/error only**: Service provides minimal state management

## Related Components

- [Event Service](event.md) - System event notifications
- [GUI Service](gui.md) - User interface control
- [Settings Service](settings.md) - Configuration management

## File Reference

- Implementation: [TerminalService.h](../../apps/EKiosk/src/Services/TerminalService.h)
- Implementation: [TerminalService.cpp](../../apps/EKiosk/src/Services/TerminalService.cpp)
- Error constants: [HookConstants.h](../../include/SDK/PaymentProcessor/Core/HookConstants.h)
