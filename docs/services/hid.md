# HID Service

The HID (Human Interface Device) Service provides basic HID device control.

## Overview

The HID Service (`IHIDService`) provides:

- HID device enable/disable control
- Data value string formatting
- Minimal device management

## Interface

```cpp
class IHIDService {
public:
    /// Enable or disable HID device
    virtual bool setEnable(bool aEnable, const QString &aDevice = QString()) = 0;

    /// Convert data value to string representation
    virtual QString valueToString(const QVariant &aData) = 0;
};
```

## Device Control

### Enable/Disable HID

```cpp
auto hidService = core->getHIDService();

if (!hidService) return;

// Enable HID device
bool enabled = hidService->setEnable(true);

// Enable specific device
enabled = hidService->setEnable(true, "keyboard");

// Disable device
bool disabled = hidService->setEnable(false);
```

## Value Formatting

### Convert Value to String

```cpp
QVariant data = 42;
QString formatted = hidService->valueToString(data);

// Or with different types
QVariant boolData = true;
QString boolStr = hidService->valueToString(boolData);
```

## Limitations

- **Minimal API**: Only enable/disable and value formatting
- **No device enumeration**: Cannot list connected devices
- **No input handling**: No keyboard/mouse event processing
- **No configuration**: Device management limited

## File Reference

- Implementation: [IHIDService.h](../../include/SDK/PaymentProcessor/Core/IHIDService.h)
