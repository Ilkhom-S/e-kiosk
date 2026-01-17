# GUI Service

The GUI Service manages the graphical user interface components and user interactions for the EKiosk system.

## Overview

The GUI Service (`IGUIService`) handles:

- UI component management and rendering
- User input processing and validation
- Screen layout and navigation
- Theme and styling management
- Accessibility features
- Multi-language UI support
- Touch and gesture handling

## Interface

```cpp
class IGUIService : public QObject {
    Q_OBJECT

public:
    enum UIState { Normal, Loading, Error, Disabled };
    enum InputType { Touch, Keyboard, Mouse, Gesture };

    struct UIComponent {
        QString id;
        QString type;        // "button", "text", "image", "form", etc.
        QRect geometry;
        QVariantMap properties;
        bool visible;
        bool enabled;
    };

    /// Show a UI screen or dialog
    virtual bool showScreen(const QString &screenId, const QVariantMap &parameters = {}) = 0;

    /// Hide current screen
    virtual bool hideScreen(const QString &screenId) = 0;

    /// Update UI component
    virtual bool updateComponent(const QString &componentId, const QVariantMap &properties) = 0;

    /// Get UI component properties
    virtual UIComponent getComponent(const QString &componentId) const = 0;

    /// Process user input
    virtual bool processInput(InputType type, const QVariantMap &inputData) = 0;

    /// Set UI theme
    virtual bool setTheme(const QString &themeName) = 0;

    /// Get current UI state
    virtual UIState getUIState() const = 0;

    /// Show loading indicator
    virtual void showLoading(const QString &message = QString()) = 0;

    /// Hide loading indicator
    virtual void hideLoading() = 0;

    /// Display error message
    virtual void showError(const QString &message, const QString &details = QString()) = 0;

    /// Display success message
    virtual void showSuccess(const QString &message) = 0;

    // ... additional methods for UI management
};
```

## Screen Management

### Showing Screens

```cpp
// Get GUI service from core
auto guiService = core->getGUIService();

if (!guiService) {
    LOG(log, LogLevel::Error, "GUI service not available");
    return;
}

// Show main menu screen
bool shown = guiService->showScreen("main_menu");

if (shown) {
    LOG(log, LogLevel::Info, "Main menu displayed");
} else {
    LOG(log, LogLevel::Error, "Failed to show main menu");
}
```

### Screen Navigation

```cpp
void navigateToPaymentScreen() {
    // Hide current screen
    guiService->hideScreen("product_selection");

    // Show payment screen with parameters
    QVariantMap paymentParams = {
        {"totalAmount", 25.99},
        {"currency", "USD"},
        {"paymentMethods", QStringList{"credit_card", "cash", "digital_wallet"}}
    };

    bool shown = guiService->showScreen("payment", paymentParams);

    if (shown) {
        LOG(log, LogLevel::Info, "Payment screen displayed");
    } else {
        LOG(log, LogLevel::Error, "Failed to show payment screen");
    }
}
```

### Screen Transitions

```cpp
void performScreenTransition(const QString &fromScreen, const QString &toScreen) {
    // Show loading during transition
    guiService->showLoading("Loading...");

    // Hide current screen
    guiService->hideScreen(fromScreen);

    // Perform any necessary data loading
    loadScreenData(toScreen);

    // Show new screen
    guiService->showScreen(toScreen);

    // Hide loading
    guiService->hideLoading();

    LOG(log, LogLevel::Info, QString("Transitioned from %1 to %2").arg(fromScreen, toScreen));
}
```

## Component Management

### Updating UI Components

```cpp
// Update product price display
QVariantMap priceProperties = {
    {"text", "$25.99"},
    {"color", "green"},
    {"fontSize", 24}
};

bool updated = guiService->updateComponent("product_price", priceProperties);

if (updated) {
    LOG(log, LogLevel::Info, "Product price updated");
} else {
    LOG(log, LogLevel::Error, "Failed to update product price");
}
```

### Dynamic Component Creation

```cpp
void addProductToCart(const QString &productName, double price) {
    // Create cart item component
    QVariantMap cartItem = {
        {"type", "cart_item"},
        {"productName", productName},
        {"price", price},
        {"quantity", 1}
    };

    // Add to cart display
    bool added = guiService->updateComponent("cart_items", cartItem);

    if (added) {
        // Update cart total
        updateCartTotal();

        LOG(log, LogLevel::Info, QString("Added to cart: %1").arg(productName));
    } else {
        LOG(log, LogLevel::Error, QString("Failed to add to cart: %1").arg(productName));
    }
}
```

### Component State Management

```cpp
void updatePaymentButtonState(bool paymentInProgress) {
    QVariantMap buttonProperties;

    if (paymentInProgress) {
        buttonProperties = {
            {"enabled", false},
            {"text", "Processing..."},
            {"style", "disabled"}
        };
    } else {
        buttonProperties = {
            {"enabled", true},
            {"text", "Pay Now"},
            {"style", "primary"}
        };
    }

    guiService->updateComponent("payment_button", buttonProperties);
}
```

## User Input Processing

### Touch Input Handling

```cpp
void handleTouchInput(const QPoint &position, bool pressed) {
    QVariantMap inputData = {
        {"type", "touch"},
        {"position", QPointF(position)},
        {"pressed", pressed},
        {"timestamp", QDateTime::currentDateTime()}
    };

    bool processed = guiService->processInput(IGUIService::Touch, inputData);

    if (processed) {
        LOG(log, LogLevel::Debug, QString("Touch input processed at (%1, %2)")
            .arg(position.x()).arg(position.y()));
    } else {
        LOG(log, LogLevel::Warning, "Touch input processing failed");
    }
}
```

### Keyboard Input

```cpp
void handleKeyboardInput(const QString &text, int keyCode) {
    QVariantMap inputData = {
        {"type", "keyboard"},
        {"text", text},
        {"keyCode", keyCode},
        {"modifiers", getKeyboardModifiers()},
        {"timestamp", QDateTime::currentDateTime()}
    };

    bool processed = guiService->processInput(IGUIService::Keyboard, inputData);

    if (processed) {
        LOG(log, LogLevel::Debug, QString("Keyboard input: %1 (key: %2)").arg(text).arg(keyCode));
    }
}
```

### Gesture Recognition

```cpp
void handleGesture(const QString &gestureType, const QVariantMap &gestureData) {
    QVariantMap inputData = {
        {"type", "gesture"},
        {"gestureType", gestureType},
        {"gestureData", gestureData},
        {"timestamp", QDateTime::currentDateTime()}
    };

    bool processed = guiService->processInput(IGUIService::Gesture, inputData);

    if (processed) {
        LOG(log, LogLevel::Info, QString("Gesture processed: %1").arg(gestureType));

        // Handle specific gestures
        if (gestureType == "swipe_left") {
            navigateToPreviousScreen();
        } else if (gestureType == "swipe_right") {
            navigateToNextScreen();
        } else if (gestureType == "pinch") {
            handleZoomGesture(gestureData);
        }
    }
}
```

## Theme and Styling

### Theme Management

```cpp
void applyTheme(const QString &themeName) {
    bool applied = guiService->setTheme(themeName);

    if (applied) {
        LOG(log, LogLevel::Info, QString("Theme applied: %1").arg(themeName));

        // Save theme preference
        auto settings = core->getSettingsService();
        settings->setValue("ui/theme", themeName);

    } else {
        LOG(log, LogLevel::Error, QString("Failed to apply theme: %1").arg(themeName));
    }
}

void initializeTheme() {
    // Load saved theme or use default
    auto settings = core->getSettingsService();
    QString themeName = settings->getValue("ui/theme", "default").toString();

    applyTheme(themeName);
}
```

### Dynamic Styling

```cpp
void updateComponentStyling() {
    // Update button styling based on state
    QVariantMap buttonStyle = {
        {"backgroundColor", isPaymentValid() ? "green" : "gray"},
        {"textColor", "white"},
        {"borderRadius", 8},
        {"fontSize", 16},
        {"padding", 12}
    };

    guiService->updateComponent("pay_button", {{"style", buttonStyle}});
}
```

## UI State Management

### Loading States

```cpp
void showLoadingDuringPayment() {
    // Show loading indicator
    guiService->showLoading("Processing payment...");

    // Disable UI interactions
    guiService->updateComponent("main_ui", {{"enabled", false}});

    // Process payment
    bool success = processPayment();

    // Hide loading
    guiService->hideLoading();

    // Re-enable UI
    guiService->updateComponent("main_ui", {{"enabled", true}});

    // Show result
    if (success) {
        guiService->showSuccess("Payment successful!");
    } else {
        guiService->showError("Payment failed", "Please try again");
    }
}
```

### Error Handling

```cpp
void handleUIError(const QString &errorMessage, const QString &errorDetails) {
    // Update UI state
    guiService->showError(errorMessage, errorDetails);

    // Log error
    LOG(log, LogLevel::Error, QString("UI Error: %1 - %2").arg(errorMessage, errorDetails));

    // Update error display component
    QVariantMap errorDisplay = {
        {"visible", true},
        {"title", "Error"},
        {"message", errorMessage},
        {"details", errorDetails},
        {"retryButton", true}
    };

    guiService->updateComponent("error_dialog", errorDisplay);
}
```

### Success Feedback

```cpp
void showPaymentSuccess() {
    // Show success message
    guiService->showSuccess("Payment completed successfully!");

    // Update receipt display
    QVariantMap receiptData = {
        {"transactionId", generateTransactionId()},
        {"amount", m_paymentAmount},
        {"timestamp", QDateTime::currentDateTime()},
        {"items", m_cartItems}
    };

    guiService->updateComponent("receipt_display", receiptData);

    // Navigate to receipt screen
    guiService->showScreen("receipt");
}
```

## Usage in Plugins

GUI Service is commonly used in user interface and interaction plugins:

```cpp
class PaymentUIPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mGUIService = mCore->getGUIService();
            mPaymentService = mCore->getPaymentService();
            mEventService = mCore->getEventService();
            mLog = kernel->getLog("PaymentUI");
        }

        return true;
    }

    void showPaymentInterface() {
        // Show payment screen
        QVariantMap paymentParams = {
            {"title", "Select Payment Method"},
            {"amount", m_currentAmount},
            {"methods", getAvailablePaymentMethods()}
        };

        bool shown = mGUIService->showScreen("payment_selection", paymentParams);

        if (shown) {
            LOG(mLog, LogLevel::Info, "Payment interface displayed");
        } else {
            LOG(mLog, LogLevel::Error, "Failed to show payment interface");
        }
    }

    void handlePaymentMethodSelection(const QString &method) {
        // Update UI to show selected method
        QVariantMap selectionUpdate = {
            {"selectedMethod", method},
            {"status", "method_selected"}
        };

        mGUIService->updateComponent("payment_selection", selectionUpdate);

        // Show method-specific interface
        if (method == "credit_card") {
            showCreditCardForm();
        } else if (method == "cash") {
            showCashPaymentInterface();
        } else if (method == "digital_wallet") {
            showDigitalWalletOptions();
        }

        LOG(mLog, LogLevel::Info, QString("Payment method selected: %1").arg(method));
    }

    void showCreditCardForm() {
        QVariantMap formConfig = {
            {"fields", QVariantList{
                QVariantMap{{"name", "card_number"}, {"type", "text"}, {"label", "Card Number"}, {"masked", true}},
                QVariantMap{{"name", "expiry"}, {"type", "text"}, {"label", "Expiry (MM/YY)"}},
                QVariantMap{{"name", "cvv"}, {"type", "text"}, {"label", "CVV"}, {"masked", true}},
                QVariantMap{{"name", "name"}, {"type", "text"}, {"label", "Cardholder Name"}}
            }},
            {"submitButton", "Process Payment"},
            {"cancelButton", "Cancel"}
        };

        mGUIService->updateComponent("payment_form", formConfig);
    }

    void processCreditCardPayment(const QVariantMap &cardData) {
        // Show loading
        mGUIService->showLoading("Processing payment...");

        try {
            // Validate card data
            if (!validateCardData(cardData)) {
                throw std::runtime_error("Invalid card data");
            }

            // Process payment
            bool success = mPaymentService->processPayment(cardData);

            if (success) {
                // Show success
                mGUIService->showSuccess("Payment successful!");

                // Navigate to receipt
                showReceipt();

                // Publish payment event
                mEventService->publish("payment.completed", {
                    {"method", "credit_card"},
                    {"amount", cardData.value("amount")},
                    {"transactionId", generateTransactionId()}
                });

            } else {
                throw std::runtime_error("Payment processing failed");
            }

        } catch (const std::exception &e) {
            LOG(mLog, LogLevel::Error, QString("Payment error: %1").arg(e.what()));

            // Show error
            mGUIService->showError("Payment Failed", e.what());

            // Reset form
            resetPaymentForm();
        }

        // Hide loading
        mGUIService->hideLoading();
    }

    void showCashPaymentInterface() {
        QVariantMap cashInterface = {
            {"amountDue", m_currentAmount},
            {"amountInserted", 0.0},
            {"changeDue", 0.0},
            {"status", "waiting_for_cash"}
        };

        mGUIService->updateComponent("cash_payment", cashInterface);

        // Start cash acceptance
        startCashAcceptance();
    }

    void handleCashInserted(double amount) {
        m_insertedAmount += amount;

        QVariantMap update = {
            {"amountInserted", m_insertedAmount},
            {"changeDue", std::max(0.0, m_insertedAmount - m_currentAmount)},
            {"status", m_insertedAmount >= m_currentAmount ? "sufficient_funds" : "waiting_for_more"}
        };

        mGUIService->updateComponent("cash_payment", update);

        if (m_insertedAmount >= m_currentAmount) {
            completeCashPayment();
        }
    }

    void completeCashPayment() {
        double changeAmount = m_insertedAmount - m_currentAmount;

        // Process payment
        bool success = mPaymentService->processCashPayment(m_currentAmount, m_insertedAmount);

        if (success) {
            if (changeAmount > 0) {
                // Dispense change
                dispenseChange(changeAmount);

                mGUIService->showSuccess(QString("Payment successful! Change: $%1").arg(changeAmount));
            } else {
                mGUIService->showSuccess("Payment successful!");
            }

            showReceipt();

        } else {
            mGUIService->showError("Cash payment failed");
        }
    }

    void showDigitalWalletOptions() {
        QStringList availableWallets = getAvailableWallets();

        QVariantMap walletOptions = {
            {"wallets", availableWallets},
            {"status", "select_wallet"}
        };

        mGUIService->updateComponent("digital_wallet_selection", walletOptions);
    }

    void handleWalletSelection(const QString &walletType) {
        // Show wallet-specific interface
        QVariantMap walletInterface = {
            {"walletType", walletType},
            {"amount", m_currentAmount},
            {"status", "ready_to_pay"}
        };

        mGUIService->updateComponent("wallet_payment", walletInterface);

        // Initiate wallet payment
        initiateWalletPayment(walletType);
    }

    void showReceipt() {
        QVariantMap receiptData = {
            {"transactionId", m_transactionId},
            {"amount", m_currentAmount},
            {"method", m_selectedMethod},
            {"timestamp", QDateTime::currentDateTime()},
            {"items", m_cartItems}
        };

        mGUIService->showScreen("receipt", receiptData);

        LOG(mLog, LogLevel::Info, QString("Receipt displayed for transaction: %1").arg(m_transactionId));
    }

    void handleUIEvents() {
        // Subscribe to UI interaction events
        mEventService->subscribe("ui.button_clicked", [this](const QVariantMap &eventData) {
            QString buttonId = eventData.value("buttonId").toString();

            if (buttonId == "pay_credit_card") {
                handlePaymentMethodSelection("credit_card");
            } else if (buttonId == "pay_cash") {
                handlePaymentMethodSelection("cash");
            } else if (buttonId == "cancel_payment") {
                cancelPayment();
            }
        });

        mEventService->subscribe("ui.form_submitted", [this](const QVariantMap &eventData) {
            QString formId = eventData.value("formId").toString();
            QVariantMap formData = eventData.value("formData").toMap();

            if (formId == "credit_card_form") {
                processCreditCardPayment(formData);
            }
        });
    }

    void cancelPayment() {
        // Reset payment state
        m_selectedMethod.clear();
        m_insertedAmount = 0.0;

        // Return to product selection
        mGUIService->showScreen("product_selection");

        LOG(mLog, LogLevel::Info, "Payment cancelled");
    }

private:
    IGUIService *mGUIService;
    IPaymentService *mPaymentService;
    IEventService *mEventService;
    ILog *mLog;

    double m_currentAmount;
    QString m_selectedMethod;
    double m_insertedAmount;
    QString m_transactionId;
    QVariantList m_cartItems;
};
```

### Exception Handling

```cpp
try {
    // Validate UI parameters
    if (screenId.isEmpty()) {
        throw std::invalid_argument("Screen ID cannot be empty");
    }

    // Check GUI service availability
    if (!guiService) {
        throw std::runtime_error("GUI service not available");
    }

    // Show screen
    if (!guiService->showScreen(screenId, parameters)) {
        throw std::runtime_error("Failed to show screen");
    }

} catch (const std::invalid_argument &e) {
    LOG(log, LogLevel::Error, QString("Invalid UI operation: %1").arg(e.what()));

} catch (const std::runtime_error &e) {
    LOG(log, LogLevel::Error, QString("GUI service error: %1").arg(e.what()));

    // Handle error - show error screen, fallback UI, etc.
    showErrorScreen(e.what());

} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Unexpected UI error: %1").arg(e.what()));
}
```

## Accessibility Features

```cpp
void enableAccessibility() {
    // Enable screen reader support
    QVariantMap accessibilitySettings = {
        {"screenReader", true},
        {"highContrast", false},
        {"largeText", false},
        {"keyboardNavigation", true}
    };

    guiService->updateComponent("accessibility", accessibilitySettings);

    // Set up keyboard shortcuts
    setupKeyboardShortcuts();

    LOG(log, LogLevel::Info, "Accessibility features enabled");
}

void announceScreenChange(const QString &screenName) {
    // Use audio service for screen announcements
    auto audioService = core->getAudioService();

    if (audioService) {
        QString announcement = QString("Now displaying: %1").arg(screenName);
        audioService->speakText(announcement);
    }

    // Update screen reader
    guiService->updateComponent("screen_reader", {{"currentScreen", screenName}});
}
```

## Multi-language Support

```cpp
void updateUILanguage(const QString &languageCode) {
    // Load language-specific strings
    QVariantMap translations = loadTranslations(languageCode);

    // Update all UI components with translated text
    guiService->updateComponent("main_menu", {
        {"title", translations.value("menu_title")},
        {"buttons", translations.value("menu_buttons")}
    });

    guiService->updateComponent("payment_screen", {
        {"title", translations.value("payment_title")},
        {"labels", translations.value("payment_labels")}
    });

    // Update theme for RTL languages if needed
    if (isRTLLanguage(languageCode)) {
        guiService->setTheme("rtl_theme");
    }

    LOG(log, LogLevel::Info, QString("UI language updated to: %1").arg(languageCode));
}
```

## Performance Considerations

- Use asynchronous UI updates for heavy operations
- Implement UI virtualization for large lists
- Cache component properties to reduce updates
- Use background loading for images and resources
- Monitor UI thread performance

## Security Considerations

- Validate all user input before processing
- Sanitize display data to prevent injection
- Implement UI access controls
- Secure sensitive data display (masking)
- Validate navigation to prevent unauthorized access

## Dependencies

- Settings Service (for UI configuration)
- Event Service (for UI event handling)
- Audio Service (for accessibility)
- Theme Service (for styling)

## See Also

- [Settings Service](settings.md) - UI configuration
- [Event Service](event.md) - UI event handling
- [Audio Service](audio.md) - Accessibility features
- [Theme Service](../../plugins/GraphicBackends/) - UI styling
