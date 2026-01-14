# Connection Module

## Purpose

The Connection module provides an abstraction layer for network connectivity management in EKiosk applications. It supports various connection types (named pipes, dial-up, TCP) and provides automatic monitoring and failover.

---

## Quick start 🔧

```cpp
#include <Connection/IConnection.h>

// Create and open a dial-up connection
auto conn = IConnection::create("MyConnection", EConnectionTypes::Dialup, networkManager, log);
conn->open();

connect(conn, &IConnection::connectionAlive, this, &MyClass::onConnectionAlive);
```

---

## Features

- Named pipes for local IPC (Windows)
- Dial-up and TCP socket support with automatic monitoring
- Event-based connection status notifications
- Platform-aware implementations (RAS on Windows)

---

## Configuration

No global settings; connection-specific options are provided by constructors and factory methods. Dial-up configuration is handled by `createDialupConnection` parameters.

---

## Usage / API highlights

- `IConnection::create(...)` — factory for creating connection instances
- `connection->open()` / `connection->close()` — lifecycle management
- Signals: `connectionAlive`, `connectionLost`, `messageReceived`

Refer to the module source README for internal APIs and contributor notes.

---

## Integration

Link against the module in your CMake target:

```cmake
target_link_libraries(MyApp PRIVATE Connection)
```

---

## Testing

Unit tests are located in `tests/modules/Connection/`. Run using the project's test target:

```bash
cmake --build build --target test -R Connection
```

---

## Migration notes

- Replace direct RAS API calls with this module for better abstraction and error handling.

---

## Further reading

- Source & implementation notes: `src/modules/Connection/README.md` (file layout and contributor guidance)"
