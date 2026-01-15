# Driver Plugins

Hardware device driver plugins.

## Available Drivers

| Plugin                            | Devices           | Status          |
| --------------------------------- | ----------------- | --------------- |
| [BillAcceptor](BillAcceptor/)     | Bill validators   | ✅ Production   |
| [BillDispensers](BillDispensers/) | Cash dispensers   | ✅ Production   |
| [CardReader](CardReader/)         | Card readers      | ✅ Production   |
| [CoinAcceptor](CoinAcceptor/)     | Coin validators   | ✅ Production   |
| [FR](FR/)                         | Fiscal registers  | ⚠️ Russia-only  |
| [HID](HID/)                       | USB HID devices   | ✅ Production   |
| [IOPort](IOPort/)                 | Port devices      | ✅ Production   |
| [Modem](Modem/)                   | Modems            | ✅ Production   |
| [Parameters](Parameters/)         | Device parameters | ✅ Utility      |
| [Printer](Printer/)               | Printers          | ✅ Production   |
| [VirtualDevices](VirtualDevices/) | Test devices      | ✅ Testing      |
| [Watchdog](Watchdog/)             | Watchdog timers   | ⚠️ Windows-only |

## Creating a Driver

### 1. Implement IDevice Interface

```cpp
#include <DriversSDK/IDevice.h>

class MyDevice : public IDevice {
    Q_OBJECT
public:
    bool initialize() override;
    bool release() override;
    DeviceStatus getStatus() const override;
    QString getDeviceId() const override;

signals:
    void statusChanged(DeviceStatus status);
    void errorOccurred(const QString& error);
};
```

### 2. Implement Specific Interface

```cpp
#include <DriversSDK/IBillAcceptor.h>

class MyBillAcceptor : public IBillAcceptor {
    Q_OBJECT
public:
    bool enableAcceptance() override;
    bool disableAcceptance() override;
    bool stackBill() override;
    bool returnBill() override;

signals:
    void billInserted(int denomination);
    void billStacked(int denomination);
};
```

### 3. Create Factory

```cpp
class MyDeviceFactory : public IDeviceFactory {
public:
    QString getPluginId() const override { return "my_device"; }
    QStringList supportedModels() const override {
        return { "Model1", "Model2" };
    }
    IDevice* createDevice(const QString& model) override {
        if (model == "Model1") return new Model1Device();
        return nullptr;
    }
};
```

### 4. CMakeLists.txt

```cmake
file(GLOB SOURCES src/*.cpp src/*.h)

tc_add_driver(my_device_driver
    SOURCES ${SOURCES}
    QT_MODULES Core SerialPort
    DEPENDS HardwareCommon DriversSDK
)
```

## Common Dependencies

- `DriversSDK` - Device interfaces
- `HardwareCommon` - Base classes
- `HardwareIOPorts` - Port communication
- `Log` - Logging

## Platform Support

| Platform | Serial | USB HID | USB libusb |
| -------- | ------ | ------- | ---------- |
| Windows  | ✅     | ✅      | ✅         |
| Linux    | 🔬     | 🔬      | 🔬         |
| macOS    | 🔬     | 🔬      | 🔬         |

## Testing

Use `VirtualDevices` plugin for testing without hardware:

```ini
[Devices]
BillAcceptor=Virtual_BillAcceptor
Printer=Virtual_Printer
```
