# EKiosk Services Documentation

This directory contains comprehensive documentation for all EKiosk core services. Services provide the fundamental functionality that plugins and the application use to interact with hardware, network, database, and other system components.

## Service Architecture

EKiosk uses a service-oriented architecture where all major functionality is exposed through well-defined service interfaces. Services are managed by the `ServiceController` and accessed through the `ICore` interface.

### Service Categories

Services are organized into the following categories:

- **Core Infrastructure**: Database, Network, Settings, Event services
- **Payment Processing**: Crypt, Payment, Funds services
- **Hardware Management**: Device, HID, Printer, Audio services
- **User Interface**: GUI, Terminal services
- **System Integration**: Remote, Scheduler, Plugin services
- **Business Logic**: Ad, Hook services

### Service Lifecycle

1. **Registration**: Services are registered with the ServiceController during application startup
2. **Initialization**: Services are initialized in dependency order
3. **Operation**: Services provide functionality to plugins and application components
4. **Shutdown**: Services are properly shut down during application termination

### Service Access

Services are accessed through the `ICore` interface:

```cpp
// Get core interface
auto core = dynamic_cast<SDK::PaymentProcessor::ICore*>(
    environment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

// Access specific service
auto cryptService = core->getCryptService();
auto networkService = core->getNetworkService();
```

## Available Services

### Core Infrastructure Services

| Service | Interface | Purpose |
|---------|-----------|---------|
| [Database Service](database.md) | `IDatabaseService` | Database connectivity and operations |
| [Network Service](network.md) | `INetworkService` | Network connectivity and HTTP communications |
| [Settings Service](settings.md) | `ISettingsService` | Configuration management |
| [Event Service](event.md) | `IEventService` | Event distribution and messaging |

### Payment Processing Services

| Service | Interface | Purpose |
|---------|-----------|---------|
| [Crypt Service](crypt.md) | `ICryptService` | Cryptographic operations and key management |
| [Payment Service](payment.md) | `IPaymentService` | Payment processing and transaction management |
| [Funds Service](funds.md) | `IFundsService` | Cash handling operations |

### Hardware Management Services

| Service | Interface | Purpose |
|---------|-----------|---------|
| [Device Service](device.md) | `IDeviceService` | Hardware device management |
| [HID Service](hid.md) | `IHIDService` | Human interface device management |
| [Printer Service](printer.md) | `IPrinterService` | Printing operations |
| [Audio Service](audio.md) | `IAudioService` | Audio playback and management |

### User Interface Services

| Service | Interface | Purpose |
|---------|-----------|---------|
| [GUI Service](gui.md) | `IGUIService` | Graphical user interface management |
| [Terminal Service](terminal.md) | `ITerminalService` | Terminal-level operations |

### System Integration Services

| Service | Interface | Purpose |
|---------|-----------|---------|
| [Remote Service](remote.md) | `IRemoteService` | Remote monitoring and control |
| [Scheduler Service](scheduler.md) | `ISchedulerService` | Task scheduling and automation |
| [Plugin Service](plugin.md) | `IService` | Plugin management |

### Business Logic Services

| Service | Interface | Purpose |
|---------|-----------|---------|
| [Ad Service](ad.md) | `IService` | Advertising content management |
| [Hook Service](hook.md) | `IService` | Extension points and hooks |

## Service Implementation Guidelines

### For Service Developers

- **Interface Design**: Services should implement clean, well-documented interfaces
- **Error Handling**: Use appropriate exception types and error codes
- **Thread Safety**: Document thread safety guarantees
- **Resource Management**: Properly manage system resources
- **Configuration**: Support runtime configuration where appropriate

### For Service Consumers (Plugins)

- **Error Handling**: Always handle `ServiceIsNotImplemented` exceptions
- **Null Checks**: Check for service availability before use
- **Resource Cleanup**: Release resources when no longer needed
- **Thread Safety**: Respect service threading constraints

### Service Dependencies

Some services have dependencies on others. The initialization order is managed by the ServiceController to ensure proper startup sequence.

## Configuration

Many services are configurable through the Settings Service. Service-specific configuration is documented in each service's documentation.

## Troubleshooting

### Common Issues

- **Service Not Available**: Check service initialization logs
- **Connection Failures**: Verify network/service configuration
- **Permission Errors**: Check user permissions and service access rights
- **Resource Conflicts**: Ensure proper resource cleanup

### Debugging

Enable service logging to troubleshoot issues:

```cpp
// Enable debug logging for a service
ILog *log = environment->getLog("ServiceName");
LOG(log, LogLevel::Debug, "Debug message");
```

## See Also

- [Plugin Documentation](../plugins/README.md): How plugins access and use services
- [Architecture Documentation](../architecture.md): Overall system architecture
- [API Reference](../../include/SDK/): Service interface definitions