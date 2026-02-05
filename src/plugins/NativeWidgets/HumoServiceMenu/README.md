# HumoServiceMenu Plugin

HumoServiceMenu native widget plugin for EKiosk - provides Humo-specific service and maintenance interface for kiosk devices.

## Purpose

The HumoServiceMenu plugin provides a specialized service interface for Humo kiosk maintenance, including:

- Humo-specific device hardware testing and diagnostics
- Network and connection management for Humo systems
- Payment system configuration for Humo
- Key management and security for Humo
- System setup and configuration for Humo
- Logs and diagnostics for Humo systems
- Humo-specific functionality

This plugin is essential for technical support, maintenance personnel, and administrators to manage and troubleshoot Humo kiosk devices.

---

## Quick start 🔧

```cpp
// Access HumoService plugin through the plugin system
#include <SDK/Plugins/IPlugin.h>
#include <SDK/Plugins/IEnvironment.h>

// Get HumoService plugin instance
SDK::Plugin::IPlugin *humoService = environment->getPlugin("humo_service");
if (humoService && humoService->isReady()) {
    // Show Humo service menu
    humoService->show();
}
```

---

## Features

### Core Functionality

- **Main Service Window**: Central interface for accessing all Humo service functions
- **Device Testing**: Comprehensive hardware device testing for Humo systems
- **Network Management**: Dial-up and network connection configuration for Humo
- **Payment System**: Payment processing configuration and testing for Humo
- **Key Management**: Security key management and configuration for Humo
- **System Setup**: First-time setup wizard and system configuration for Humo
- **Diagnostics**: System diagnostics and logging for Humo systems
- **Humo-specific Features**: Specialized functionality for Humo kiosk operations
