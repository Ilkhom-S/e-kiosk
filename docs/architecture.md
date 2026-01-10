# Architecture

## Overview

EKiosk is designed with a modular architecture to separate concerns and facilitate maintenance of a complex kiosk system involving hardware devices, network communications, and user interfaces.

## Layers

### UI Layer

- **Purpose**: Handles user interactions and displays.
- **Components**: `MainWindow`, various dialogs in `src/ui/`.
- **Responsibilities**: Present information, capture user input, trigger business logic.

### Modules Layer

- **Purpose**: Implements business logic independent of UI and hardware.
- **Components**: Classes like `AuthRequest`, `GetBalanceAgent`, `SendOtp` in `src/modules/`.
- **Responsibilities**: Process requests, validate data, prepare for device/network operations.

### Devices Layer

- **Purpose**: Abstracts hardware interactions.
- **Components**: Abstract bases (e.g., `AbstractPrinter`) and implementations (e.g., `CitizenCBM1000`) in `src/devices/`.
- **Responsibilities**: Communicate with physical devices via serial ports, handle device-specific protocols.

### Connection Layer

- **Purpose**: Manages network and dial-up connections.
- **Components**: `Connect` for network, `RasConnection` for RAS in `src/connection/`.
- **Responsibilities**: Establish connections, send/receive data.

### Utilities

- **Purpose**: Shared functionality.
- **Components**: Logging, receipts, system info in `src/other/`.
- **Responsibilities**: Logging, data formatting, system queries.

## Data Flow

1. User action in UI triggers module call.
2. Module prepares request and invokes device/connection.
3. Device/connection executes operation and returns result.
4. Result propagates back via signals/slots to UI.

## Design Decisions

- **Modularity**: Allows independent development and testing of components.
- **Abstraction**: Device layer uses inheritance for different hardware models.
- **Signals/Slots**: For loose coupling between components.
- **No External Config**: Constants in code for simplicity.
