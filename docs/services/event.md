# Event Service

The Event Service provides inter-component event distribution for the EKiosk system.

## Overview

The Event Service (`IEventService`) handles:

- Event publishing and distribution
- Component subscription to events
- Event-driven notifications
- Inter-service communication

## Current Implementation

The EventService uses Qt signals/slots mechanism for event delivery and provides an
enumerated set of predefined event types (EEventType).

**Status**: String-based event types are NOT supported. Use predefined enum values only.

## Interface

```cpp
class IEventService {
public:
    /// Send event with structured data
    virtual void sendEvent(const Event &aEvent) = 0;

    /// Send event by type with variant data
    virtual void sendEvent(EEventType::Enum aType,
                          const QVariant &aData) = 0;

    /// Subscribe to all events
    virtual void subscribe(const QObject *aObject,
                          const char *aSlot) = 0;

    /// Unsubscribe from events
    virtual void unsubscribe(const QObject *aObject,
                            const char *aSlot) = 0;
};
```

## Event Subscription

### Basic Subscription

```cpp
// Get event service from core
auto eventService = core->getEventService();

if (!eventService) {
    LOG(log, LogLevel::Error, "Event service not available");
    return;
}

// Subscribe to ALL events
// Slot signature must be: void onEvent(const Event &event);
eventService->subscribe(this, SLOT(onSystemEvent(const Event&)));
```

### Event Handler Signature

```cpp
class MyComponent : public QObject {
    Q_OBJECT

    // Qt signal-slot syntax required
public slots:
    void onSystemEvent(const Event &event) {
        // Handle all events
        qDebug() << "Event received:" << event.type;
    }
};
```

### Unsubscription

```cpp
// Unsubscribe from events
eventService->unsubscribe(this, SLOT(onSystemEvent(const Event&)));
```

## Send Events

### Sending Events by Type

```cpp
// Get event service
auto eventService = core->getEventService();

// Send event with data variant
QVariant eventData = QVariantMap{
    {"status", "processing"},
    {"amount", 100.00}
};

eventService->sendEvent(EEventType::PaymentStatusChanged, eventData);
```

### Event Structure

```cpp
// Event type enumeration (SDK/PaymentProcessor/Core/Event.h)
namespace EEventType {
    enum Enum {
        // Payment events
        PaymentStarted,
        PaymentCompleted,
        PaymentFailed,
        // Device events
        DeviceConnected,
        DeviceDisconnected,
        // Terminal events
        TerminalStatusChanged,
        // ... and more predefined types
    };
}
```

## Limitations

- **No string-based filtering**: Events cannot be filtered by custom string names
- **No event persistence**: Events are not stored or replayed
- **Qt slots required**: Must use Qt signal-slot syntax (Q_OBJECT, slots:, etc.)
- **Broad subscription**: Cannot subscribe to specific event types; subscribes to ALL events
- **No event history**: No method to query past events

## Design Pattern

Events follow the observer pattern:
1. Component calls `subscribe()` to register Qt slot
2. Event Service maintains list of subscribers
3. When `sendEvent()` is called, Service invokes all registered slots
4. Subscribers receive events via their slot handler
