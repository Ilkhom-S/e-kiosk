# Connection

Inter-process communication module.

## Purpose

Provides IPC between applications:

- Named pipes (Windows)
- TCP/IP sockets
- Message passing

## Structure (implementation)

```text
src/modules/Connection/
├── CMakeLists.txt          # Build configuration
├── Common/                 # Shared connection implementations
│   ├── ConnectionBase.cpp  # Base connection implementation
│   └── ConnectionBase.h    # Base connection interface
├── Win32/                  # Windows-specific code
│   └── src/
│       ├── Common.cpp      # Common connection utilities
│       ├── DialupConnection.cpp  # Dial-up connection implementation
│       ├── DialupConnection.h    # Dial-up connection interface
│       ├── LocalConnection.cpp   # Local network connection
│       ├── LocalConnection.h     # Local connection interface
│       ├── RasWrapper.cpp        # RAS API wrapper
│       └── RasWrapper.h          # RAS wrapper interface
└── tests/                   # Unit tests

include/Connection/
├── IConnection.h          # Main connection interface
└── NetworkError.h         # Network-specific exceptions
```

**Contributor notes:**

- Keep high-level documentation and usage examples in `docs/modules/connection.md`.
- Use this README for implementation notes, file layout and platform-specific guidance.

## Usage

```cpp
#include "Connection/ConnectionManager.h"

// Server side
ConnectionServer* server = new ConnectionServer("TerminalChannel");
connect(server, &ConnectionServer::messageReceived,
        this, &MyClass::onMessage);
server->start();

// Client side
ConnectionClient* client = new ConnectionClient("TerminalChannel");
client->connect();
client->sendMessage("STATUS_REQUEST");
```

## Communication Types

### Named Pipes (Windows)

```cpp
// High-performance local IPC
PipeConnection pipe("\\\\.\\pipe\\TerminalPipe");
pipe.open();
pipe.write(data);
```

### TCP Sockets

```cpp
// Cross-platform network IPC
TcpConnection tcp("127.0.0.1", 9000);
tcp.connect();
tcp.send(message);
```

## Key Files

| File                  | Purpose              |
| --------------------- | -------------------- |
| `ConnectionManager.h` | Main manager         |
| `IConnection.h`       | Connection interface |
| `PipeConnection.cpp`  | Named pipes          |
| `TcpConnection.cpp`   | TCP sockets          |

## Message Protocol

```mermaid
packet-beta
    0-31: "Length (4 bytes)"
    32-47: "Type (2 bytes)"
    48-95: "Payload (N bytes)"
```

## Dependencies

- Qt Network module
- `Log` module

## Platform Support

| Feature      | Windows | Linux   | macOS   |
| ------------ | ------- | ------- | ------- |
| Named Pipes  | ✅      | ❌      | ❌      |
| Unix Sockets | ❌      | 🔬 TODO | 🔬 TODO |
| TCP Sockets  | ✅      | ✅      | ✅      |

## Migration TODO

For Linux/macOS:

- [ ] Replace named pipes with Unix domain sockets
- [ ] Create `UnixSocketConnection` class
- [ ] Update `ConnectionManager` for platform detection
