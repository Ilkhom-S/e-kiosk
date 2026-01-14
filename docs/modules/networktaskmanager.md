# NetworkTaskManager Module

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
```

---

## Features

- Request queue and automatic retries
- Connection pooling and timeouts
- SSL/TLS configuration and certificate validation
- Request/response logging integration

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

## Usage / API highlights

- `NetworkTaskManager::instance()` — obtain the manager
- `ntm->get(url)` / `ntm->post(url, data)` — create tasks
- `NetworkTask` signals and methods for response handling

---

## Integration

```cmake
target_link_libraries(MyApp PRIVATE NetworkTaskManager)
```

Unit tests live under `tests/modules/NetworkTaskManager/`.

---

## Migration notes (Qt6)

- `QNetworkReply::error()` (Qt5) => `QNetworkReply::errorOccurred` (Qt6)

---

## Further reading

- Implementation & layout: `src/modules/NetworkTaskManager/README.md` (internal notes and contributor guidance)
