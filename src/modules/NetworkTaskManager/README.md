# NetworkTaskManager

HTTP/HTTPS network operations module.

## Purpose

Handles all network communication:

- HTTP/HTTPS requests
- Request queuing and retry
- SSL/TLS configuration
- Response handling

## Usage

```cpp
#include "NetworkTaskManager/NetworkTaskManager.h"

NetworkTaskManager* ntm = NetworkTaskManager::instance();

// Simple GET request
NetworkTask* task = ntm->get(QUrl("https://api.example.com/data"));
connect(task, &NetworkTask::finished, this, &MyClass::onResponse);

// POST with data
QByteArray postData = createRequest();
NetworkTask* task = ntm->post(url, postData);
task->setHeader("Content-Type", "application/json");
```

## Key Files

| File                   | Purpose               |
| ---------------------- | --------------------- |
| `NetworkTaskManager.h` | Main manager class    |
| `NetworkTask.h`        | Individual task class |
| `SslConfiguration.h`   | SSL settings          |

## Features

- Automatic retry on failure
- Request queue management
- Connection pooling
- Timeout handling
- SSL certificate validation

## Configuration

```ini
[Network]
Timeout=30000
RetryCount=3
VerifySsl=true
ProxyHost=
ProxyPort=
```

## Dependencies

- Qt Network module
- `Log` module
- `SettingsManager` module

## Platform Support

| Platform | Status  |
| -------- | ------- |
| Windows  | ✅ Full |
| Linux    | ✅ Full |
| macOS    | ✅ Full |

## Qt6 Migration Notes

```cpp
// Qt5
connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), ...);

// Qt6
connect(reply, &QNetworkReply::errorOccurred, ...);
```
