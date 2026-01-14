# SDK

Interface definitions and plugin infrastructure.

## Purpose

Contains all SDK modules:

- `PPSDK` - Payment Processor SDK
- `DriversSDK` - Hardware driver interfaces
- `GUISDK` - Graphics interfaces
- `PluginsSDK` - Plugin loading

## Structure

```
SDK/
├── PPSDK/           # Payment interfaces
├── DriversSDK/      # Driver interfaces
├── GUISDK/          # Graphics interfaces
└── PluginsSDK/      # Plugin loading
```

## PPSDK

Payment Processor SDK - core payment interfaces.

```cpp
#include <PPSDK/IPaymentProvider.h>

class MyPaymentProvider : public IPaymentProvider {
    bool processPayment(const PaymentRequest& req) override;
    PaymentStatus getStatus() const override;
};
```

## DriversSDK

Hardware driver interfaces.

```cpp
#include <DriversSDK/IDevice.h>
#include <DriversSDK/IBillAcceptor.h>

class MyBillAcceptor : public IBillAcceptor {
    bool enableAcceptance() override;
    void onBillInserted(int denomination) override;
};
```

### Device Interfaces

| Interface       | Purpose               |
| --------------- | --------------------- |
| `IDevice`       | Base device interface |
| `IBillAcceptor` | Bill validators       |
| `ICoinAcceptor` | Coin validators       |
| `IPrinter`      | Receipt printers      |
| `ICardReader`   | Card readers          |
| `IModem`        | Modems                |
| `IWatchdog`     | Watchdog timers       |

## GUISDK

Graphics/UI interfaces.

```cpp
#include <GUISDK/IGraphicBackend.h>

class MyBackend : public IGraphicBackend {
    void displayScreen(const QString& name) override;
    void showWidget(const QString& id) override;
};
```

## PluginsSDK

Plugin loading infrastructure.

```cpp
#include <PluginsSDK/IPluginFactory.h>

class MyPluginFactory : public IPluginFactory {
    QString getPluginId() const override;
    QObject* createPlugin() override;
};

// Export function
extern "C" Q_DECL_EXPORT IPluginFactory* createPluginFactory();
```

## Platform Support

| Module     | Windows | Linux | macOS |
| ---------- | ------- | ----- | ----- |
| PPSDK      | ✅      | ✅    | ✅    |
| DriversSDK | ✅      | ✅    | ✅    |
| GUISDK     | ✅      | ✅    | ✅    |
| PluginsSDK | ✅      | 🔬    | 🔬    |

## Dependencies

- Qt Core module
- `Log` module
