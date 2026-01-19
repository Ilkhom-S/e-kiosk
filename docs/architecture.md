# EKiosk Architecture Overview

EKiosk is a modular Qt C++ project for self-service kiosks. The architecture follows a service-oriented design with clear separation of concerns, modular components, and a comprehensive plugin system.

## Documentation Index

- [Getting Started](getting-started.md)
- [Build Guide](build-guide.md)
- [Coding Standards](coding-standards.md)
- [Platform Compatibility](platform-compatibility.md)
- [Multilanguage Support](multilanguage.md)
- [Services Documentation](services/README.md)
- [Plugin System Architecture](plugins/README.md)
- [Migration Plan & TODO](migration-todo.md)

## Key Architectural Concepts

### Service-Oriented Architecture

EKiosk uses a service-oriented architecture where all major functionality is exposed through well-defined service interfaces. Services are managed by the `ServiceController` and accessed through the `ICore` interface.

**Service Categories:**

- **Core Infrastructure**: Database, Network, Settings, Event services
- **Payment Processing**: Crypt, Payment, Funds services
- **Hardware Management**: Device, HID, Printer, Audio services
- **User Interface**: GUI, Terminal services
- **System Integration**: Remote, Scheduler, Plugin services
- **Business Logic**: Ad, Hook services

### Plugin System

EKiosk features a comprehensive plugin system based on Qt plugins with custom factory interfaces:

- **Plugin Architecture**: All plugins implement the `SDK::Plugin::IPluginFactory` interface
- **Plugin Categories**: GraphicBackends, Payments, Drivers, NativeScenarios
- **Plugin Lifecycle**: Discovery, registration, instantiation, initialization, operation, shutdown
- **Inter-Plugin Communication**: Message passing and interface sharing through the Plugin Service

### Modular Design

- **Layered Architecture**: UI, business logic, services, hardware abstraction, utilities
- **Shared Modules**: Reusable components in `src/modules/` with public headers in `include/`
- **Application Structure**: Multiple executables in `apps/` with per-app modular structure
- **Test Organization**: Mirrors source structure in `tests/` for comprehensive coverage

## Application Structure (2026)

### Applications in `apps/`

- `kiosk`: Main kiosk application with full service integration
- `Updater`: Automatic software update manager
- `UpdaterSplashScreen`: Update splash screen display
- `WatchService`: Windows service for system monitoring
- `WatchServiceController`: System tray controller for WatchService

### Modular CMake Setup

- Root `CMakeLists.txt` orchestrates the build
- `apps/CMakeLists.txt` adds each application subdirectory
- Each app has its own `CMakeLists.txt` with `ek_add_application()`
- Shared modules use `ek_add_library()` with proper dependencies
- Plugins use `ek_add_plugin()` for consistent plugin building

## Core Components

### ScenarioEngine: JS Scripting Core

The ScenarioEngine module provides JavaScript scenario execution for all apps, modules, and plugins. It is now compatible with both Qt5 (QtScript) and Qt6 (QJSEngine), ensuring cross-version scripting support.

- All JS scripting logic is centralized in ScenarioEngine
- Qt5: QScriptEngine/QScriptValue
- Qt6: QJSEngine/QJSValue
- No abstraction needed for consumers; migration is transparent

### Service Layer

The service layer provides the fundamental functionality:

```cpp
// Service access through ICore interface
auto core = dynamic_cast<SDK::PaymentProcessor::ICore*>(
    environment->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

// Access specific services
auto databaseService = core->getDatabaseService();
auto paymentService = core->getPaymentService();
auto pluginService = core->getPluginService();
```

### Module System

Shared modules provide reusable functionality:

- **BaseApplication**: Qt application base class with common functionality
- **DebugUtils**: Cross-platform stack tracing and debugging utilities
- **Connection**: Network connectivity and connection management
- **NetworkTaskManager**: HTTP client with task queuing and retry logic
- **Logging**: Structured logging with multiple output targets
- **SysUtils**: System utilities and platform-specific operations
- **UpdateEngine**: Software update management and deployment
- **Hardware**: Hardware abstraction layer for device management
- **CryptEngine**: Cryptographic operations and key management

### Plugin Framework

Plugins extend EKiosk functionality:

```cpp
class MyPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        // Access services through kernel
        mDatabase = kernel->getDatabaseService();
        mEventService = kernel->getEventService();
        return true;
    }
};
```

## Directory Structure (2026)

```text
ekiosk/
├── apps/                    # Executable applications
│   ├── kiosk/              # Main kiosk application
│   │   ├── src/            # Application-specific code
│   │   └── CMakeLists.txt  # Application build config
│   ├── Updater/            # Update manager
│   └── ...
├── src/                     # Shared modules and libraries
│   └── modules/            # Modular components
│       ├── common/         # BaseApplication, DebugUtils
│       ├── connection/     # Connection management
│       ├── network/        # NetworkTaskManager
│       └── ...
├── include/                 # Public headers for modules
├── thirdparty/              # Vendored dependencies
│   ├── SingleApplication/  # Qt single application
│   ├── libusb/            # USB device support
│   └── ...
├── tests/                   # Unit and integration tests
│   ├── modules/           # Module-specific tests
│   └── apps/              # Application tests
├── docs/                    # Documentation
│   ├── services/          # Service documentation
│   ├── modules/           # Module documentation
│   └── plugins/           # Plugin documentation
├── plugins/                 # Plugin implementations
│   ├── GraphicBackends/   # UI rendering plugins
│   ├── Payments/          # Payment processor plugins
│   └── ...
└── cmake/                  # Build system helpers
```

## Service Integration

Services work together to provide comprehensive functionality:

- **Database Service** stores configuration and transaction data
- **Network Service** handles external communications
- **Event Service** coordinates inter-component messaging
- **Plugin Service** manages plugin lifecycle and communication
- **Payment Services** process transactions and manage funds
- **Hardware Services** interface with peripherals
- **UI Services** manage user interaction

## Platform Support

- **Windows 7**: Qt 5.15 LTS (VC toolset 142) - transitional support
- **Windows 10/11**: Qt 6.8 LTS (MSVC, Ninja)
- **Linux**: Qt 6.8 LTS (GCC, Clang, Ninja)

## Development Workflow

1. **Service Development**: Implement services with clean interfaces
2. **Plugin Development**: Create plugins using service interfaces
3. **Application Assembly**: Combine services and plugins in applications
4. **Testing**: Comprehensive unit and integration testing
5. **Deployment**: Platform-specific builds and packaging

---

**For any architectural changes, update this document and related documentation to maintain clarity.**

_This architecture supports scalable, maintainable kiosk applications with comprehensive service integration._
