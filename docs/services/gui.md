# GUI Service

The GUI Service manages widget display, popups, modals, and graphical interface rendering.

## Overview

The GUI Service (`IGUIService`) provides:

- Widget display and management
- Popup window handling
- Modal dialogs
- Screen information and screenshots
- UI settings retrieval
- Advertisement source management
- Backend object access for scenarios

## Interface

```cpp
class IGUIService {
public:
    /// Display a widget with parameters
    virtual bool show(const QString &aWidget, const QVariantMap &aParameters) = 0;

    /// Display a popup widget (above current layer)
    virtual bool showPopup(const QString &aWidget, const QVariantMap &aParameters) = 0;

    /// Display modal widget (blocking)
    virtual QVariantMap showModal(const QString &aWidget, 
                                   const QVariantMap &aParameters) = 0;

    /// Hide current popup or modal
    virtual bool hidePopup(const QVariantMap &aParameters = QVariantMap()) = 0;

    /// Notify widget of event
    virtual void notify(const QString &aEvent, const QVariantMap &aParameters) = 0;

    /// Check if GUI is disabled
    virtual bool isDisabled() const = 0;

    /// Clear all graphics items and scene
    virtual void reset() = 0;

    /// Get screen size in pixels
    virtual QRect getScreenSize(int aIndex) const = 0;

    /// Capture screenshot of current display
    virtual QPixmap getScreenshot() = 0;

    /// Get UI settings from interface.ini
    virtual QVariantMap getUiSettings(const QString &aSection) const = 0;

    /// Get advertisement source
    virtual SDK::GUI::IAdSource *getAdSource() const = 0;

    /// Get backend scenario object for scripting
    virtual QObject *getBackendObject(const QString &aName) const = 0;
};
```

## Widget Display

### Show Widget

```cpp
// Get GUI service
auto guiService = core->getGUIService();

if (!guiService) {
    LOG(log, LogLevel::Error, "GUI service not available");
    return;
}

// Show widget with parameters
QVariantMap params = {
    {"title", "Payment Complete"},
    {"amount", 5000},
    {"currency", "RUB"}
};

bool shown = guiService->show("payment_success", params);
```

### Show Popup

```cpp
// Show popup above current widget
QVariantMap popupParams = {
    {"message", "Confirm operation?"},
    {"type", "confirmation"}
};

bool shown = guiService->showPopup("confirm_dialog", popupParams);
```

### Show Modal Dialog

```cpp
// Show blocking modal dialog
QVariantMap modalParams = {
    {"title", "Enter PIN"},
    {"inputType", "numeric"},
    {"maxLength", 4}
};

QVariantMap result = guiService->showModal("pin_entry", modalParams);
```

### Hide Popup/Modal

```cpp
// Hide current popup or modal
guiService->hidePopup({{"reason", "transaction_complete"}});
```

## Screen Management

### Get Screen Size

```cpp
// Get screen dimensions
QRect screenSize = guiService->getScreenSize(0);

if (!screenSize.isEmpty()) {
    LOG(log, LogLevel::Info, 
        QString("Screen: %1x%2").arg(screenSize.width(), screenSize.height()));
}
```

### Capture Screenshot

```cpp
// Take screenshot
QPixmap screenshot = guiService->getScreenshot();

if (!screenshot.isNull()) {
    screenshot.save("/tmp/screenshot.png");
}
```

## Limitations

- **No direct component manipulation**: Cannot access individual UI components
- **No dynamic layout**: Layout is predefined in scenarios
- **Widget-based only**: Display through named widgets only
- **Single modal at a time**: Only one modal allowed (blocking)

## File Reference

- Implementation: [IGUIService.h](../../include/SDK/PaymentProcessor/Core/IGUIService.h)
