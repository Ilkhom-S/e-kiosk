# Connection Module

## Purpose

The Connection module provides an abstraction layer for network connectivity management in EKiosk applications. It supports various connection types including dial-up, local network, and remote connections, with automatic monitoring and failover capabilities.

## Implementation layout

For implementation details and file layout, see `src/modules/Connection/README.md` (this contains the file tree and contributor notes).

## Dependencies

- **Qt Core**: For QObject, QString, networking utilities
- **Windows RAS APIs**: For dial-up connection management
- **NetworkTaskManager**: For asynchronous network operations
- **ILog**: For logging connection events

## Platform Compatibility

- **Windows**: Fully supported (RAS, network interfaces)
- **Linux/macOS**: Not supported (Windows-specific RAS APIs)

## Usage

### Creating a Connection

```cpp
#include <Connection/IConnection.h>

IConnection *connection = IConnection::create("MyConnection", EConnectionTypes::Dialup, networkManager, log);
connection->open();
```

### Monitoring Connection Status

```cpp
connect(connection, &IConnection::connectionLost, this, &MyClass::onConnectionLost);
connect(connection, &IConnection::connectionAlive, this, &MyClass::onConnectionAlive);

if (connection->isConnected()) {
    // Connection is active
}
```

### Dial-up Connection Setup

```cpp
// Create dial-up connection programmatically
IConnection::createDialupConnection("MyISP", "555-1234", "username", "password", "modem0");
```

## CMake Integration

Add to your application's CMakeLists.txt:

```cmake
target_link_libraries(MyApp PRIVATE Connection)
```

## Testing

Unit tests are located in `tests/modules/Connection/`. Run with:

```bash
cmake --build build --target test
```

## Migration Notes

Replace direct RAS API calls with this module for better abstraction and error handling. The module provides a unified interface across different connection types.
