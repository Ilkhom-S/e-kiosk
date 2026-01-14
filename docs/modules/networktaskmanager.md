# NetworkTaskManager Module ✅

## Purpose

The NetworkTaskManager module provides a centralized, thread-safe layer for HTTP/HTTPS network operations used by EKiosk applications: request queuing, retries, SSL configuration, timeouts, and response processing.

---

## Quick start 🔧

```cpp
#include "NetworkTaskManager/NetworkTaskManager.h"

auto ntm = NetworkTaskManager::instance();

// Simple GET request
NetworkTask* task = ntm->get(QUrl("https://api.example.com/data"));
connect(task, &NetworkTask::finished, this, &MyClass::onResponse);

// POST with data
QByteArray postData = createRequest();
NetworkTask* post = ntm->post(url, postData);
post->setHeader("Content-Type", "application/json");
```

> Tip: Prefer high-level `NetworkTask` APIs over direct `QNetworkAccessManager` calls for consistent retry, logging and SSL handling. 💡

---

## Features

- Request queue and automatic retries
- Connection pooling and timeouts
- SSL/TLS configuration and certificate validation
- Request/response logging integration
- Thread-safe operations suitable for background work

---

## Configuration

The module reads settings from the `SettingsManager` under a `[Network]` section. Typical settings:

```ini
[Network]
Timeout=30000
RetryCount=3
VerifySsl=true
ProxyHost=
ProxyPort=
```

---

## Key files

| File                   | Purpose                               |
| ---------------------- | ------------------------------------- |
| `NetworkTaskManager.h` | Main singleton manager                |
| `NetworkTask.h`        | Represents an individual network task |
| `SslConfiguration.h`   | SSL and certificate settings          |

---

## Integration

Link against the module in your CMake target:

```cmake
target_link_libraries(MyApp PRIVATE NetworkTaskManager)
```

Unit tests live under `tests/modules/NetworkTaskManager/`.

---

## Migration notes (Qt6)

- `QNetworkReply::error()` (Qt5) => `QNetworkReply::errorOccurred` (Qt6)

---

## Source & Further reading

- Canonical docs: `docs/modules/networktaskmanager.md` (this file)
- Module source: [`src/modules/NetworkTaskManager/README.md`](../../src/modules/NetworkTaskManager/README.md)

---

_If you'd like, I can add examples, sequence diagrams, or an API table to this page._
