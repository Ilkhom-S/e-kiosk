# Hook Service

The Hook Service provides a mechanism for plugins to hook into system methods via Qt metainvocation.

## Overview

The Hook Service (`HookService`) enables:

- Plugin method invocation
- Hook plugins registration
- Qt metainvocation mechanism for dynamic method calls

## Current Implementation - LIMITED INTERFACE

**IMPORTANT**: The official `IHookService` interface is NOT EXPOSED (see commented-out
interface in HookService.h marked "TODO"). The service provides only internal method invocation.

The internal implementation is rudimentary and subject to change.

## Architecture

```cpp
class HookService : public QObject, public IService {
public:
    /// Invoke method on all registered hook plugins
    virtual bool invokeHook(
        const QString &aMethodName,
        QGenericArgument aVal0 = QGenericArgument(0),
        QGenericArgument aVal1 = QGenericArgument(),
        // ... up to 10 arguments
        QGenericArgument aVal9 = QGenericArgument()
    );
};
```

## Hook Plugin Registration

Plugins are registered internally by the system. Hook plugins must:

1. Implement specific hook methods
2. Be registered as hook plugins via SDK plugin interface
3. Have methods with Qt metainvocation-compatible signatures

## Method Invocation

```cpp
// Get hook service
auto hookService = core->getHookService();

// Invoke a method on all registered hook plugins
hookService->invokeHook("onPaymentStarted", Q_ARG(double, 100.00));

// Invoke with multiple arguments
hookService->invokeHook("onTransactionEvent",
    Q_ARG(QString, "completed"),
    Q_ARG(QVariantMap, data));
```

## Limitations

- **NOT a public API**: Interface is not officially exposed in SDK
- **No callback registration**: Cannot dynamically register hooks from plugins
- **No priorities or ordering**: Hooks execute in registration order
- **No async execution**: All invocations are synchronous
- **Qt metainvocation required**: Method signatures must be compatible with Qt's metainvocation
- **No return value handling**: Multiple hook plugins cannot return values
- **Subject to change**: No stability guarantees; may be redesigned

## Current Hook Plugins

Hook plugins are registered internally and invoke predefined methods. The list of available
hooks and their signatures is NOT public API.

## Future Plans

The commented-out interface in the code suggests future plans for:

- Proper IHookService interface exposure
- Named hook registration
- Return value handling
- Async execution

See source code comment: "TODO: Возможно этот интерфейс потом вынесем в SDK"
(TODO: Maybe we'll move this interface to SDK later)

## Related Usage

- [Plugin Service](plugin.md) - Plugin system management
- [Event Service](event.md) - System event distribution
- [Service Architecture](../architecture.md) - Overall architecture

## File Reference

- Implementation: [HookService.h](../../apps/EKiosk/src/Services/HookService.h)
- Implementation: [HookService.cpp](../../apps/EKiosk/src/Services/HookService.cpp)
- Hook constants: [HookConstants.h](../../include/SDK/PaymentProcessor/Core/HookConstants.h)
