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

- Request types: **GET**, **POST**, **PUT**, **HEAD**
- Request queue with synchronous (blocking) and asynchronous modes
- Flags to control behavior: **BlockingMode**, **Continue** (resume), **IgnoreErrors**
- Resumable file downloads via `FileDownloadTask` + `FileDataStream` (supports continue/partial downloads)
- Data stream types: `FileDataStream`, `MemoryDataStream`, and custom `DataStream` implementations
- Content verification via `IVerifier` (provided `Md5Verifier`, `Sha256Verifier` implementations)
- Per-task timeouts and timeout handling
- Download speed limiting (`setDownloadSpeedLimit`) to throttle bandwidth
- Proxy support (`setProxy`) and User-Agent configuration (`setUserAgent` / `getUserAgent`)
- Access to request and response headers
- Progress and completion signals: `onProgress`, upload/download progress, `onComplete`
- SSL certificate handling and SSL error callbacks
- Connection pooling and reuse via an internal `QNetworkAccessManager`
- Ability to add/remove/clear tasks and wait for a task to finish (`waitForFinished`)
- Rich error reporting including verification states (`VerifyFailed`, `TaskFailedButVerified`)
- Integration with the `Log` subsystem for request/response logging

---

## Platform support

| Platform | Status  | Notes                                                            |
| -------- | ------- | ---------------------------------------------------------------- |
| Windows  | ✅ Full | Qt Network support; SSL/Cert handling available                  |
| Linux    | ✅ Full | Qt Network support; may require openssl installed                |
| macOS    | ✅ Full | Qt Network support; certificates handled via platform mechanisms |

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
- `ntm->addTask(task)` / `ntm->removeTask(task)` — manage tasks
- `ntm->setProxy(...)`, `ntm->setDownloadSpeedLimit(...)`, `ntm->setUserAgent(...)` — global configuration
- `NetworkTask` API: set type (Get/Post/Put), setUrl, setTimeout, setFlags (BlockingMode/Continue/IgnoreErrors), setVerifier, setDataStream
- Signals: `onProgress(qint64,current, qint64 total)`, `onComplete()`; manager emits `networkTaskStatus(bool failure)` for network failures

---

### Example: resumable file download with verification

```cpp
#include "NetworkTaskManager/FileDownloadTask.h"
#include "NetworkTaskManager/NetworkTaskManager.h"
#include "NetworkTaskManager/hashVerifier.h" // Md5Verifier/Sha256Verifier

FileDownloadTask task(QUrl("https://example.com/file.bin"), "C:/tmp/file.bin");

// Resume if partial file exists
task.setFlags(NetworkTask::Continue);
// Verify downloaded content
task.setVerifier(new Md5Verifier("expected-md5-hex"));
// Per-task timeout
task.setTimeout(60 * 1000); // 60s

// Optional: run in blocking mode (manager will process and wait)
// task.setFlags(NetworkTask::Flags(task.getFlags() | NetworkTask::BlockingMode));

// Add task to manager
auto ntm = NetworkTaskManager::instance();
ntm->addTask(&task);

// Optional: wait synchronously for completion
task.waitForFinished();

if (task.getError() == NetworkTask::NoError) {
    // success
} else if (task.getError() == NetworkTask::VerifyFailed) {
    qWarning() << "Download succeeded but verification failed";
} else {
    qWarning() << "Download failed:" << task.errorString();
}
```

---

### Example: observe progress and speed limits

```cpp
#include "NetworkTaskManager/FileDownloadTask.h"
#include "NetworkTaskManager/NetworkTaskManager.h"

FileDownloadTask task(QUrl("https://example.com/large.bin"), "C:/tmp/large.bin");

auto ntm = NetworkTaskManager::instance();
// Limit download speed to 50% of max
ntm->setDownloadSpeedLimit(50);

connect(&task, &NetworkTask::onProgress, [](qint64 current, qint64 total){
    qDebug() << "Progress:" << current << "/" << total << "bytes";
});

ntm->addTask(&task);
```

---

### Example: custom verifier and data stream (memory-based)

```cpp
#include "NetworkTaskManager/NetworkTaskManager.h"
#include "NetworkTaskManager/NetworkTask.h"
#include "NetworkTaskManager/MemoryDataStream.h"

NetworkTask task;

// Use a custom in-memory stream and verifier
task.setDataStream(new MemoryDataStream());
task.setVerifier(new Sha256Verifier("expected-sha256-hex"));

task.setUrl(QUrl("https://api.example.com/data.bin"));

auto ntm = NetworkTaskManager::instance();
ntm->addTask(&task);

connect(&task, &NetworkTask::onComplete, [&task]{
    if (task.getError()==NetworkTask::NoError) {
        QByteArray data = task.getDataStream()->readAll();
        // process data
    }
});
```

---

### Example: set proxy, user-agent and custom headers

```cpp
#include <QtNetwork/QNetworkProxy>

auto ntm = NetworkTaskManager::instance();
QNetworkProxy proxy(QNetworkProxy::HttpProxy, "proxy.example.com", 8080);
ntm->setProxy(proxy);
ntm->setUserAgent("EKiosk/1.0");

FileDownloadTask task(QUrl("https://example.com/file"), "C:/tmp/file");
// Add a custom header
task.getRequestHeader()["Authorization"] = "Bearer <token>";

ntm->addTask(&task);
```

---

## Integration

```cmake
target_link_libraries(MyApp PRIVATE NetworkTaskManager)
```

---

## Migration notes (Qt6)

- `QNetworkReply::error()` (Qt5) => `QNetworkReply::errorOccurred` (Qt6)

## Testing

- Unit tests: `tests/modules/NetworkTaskManager/` — e.g., `TestThread.cpp` demonstrates a FileDownloadTask download and completion check.
- Run tests using the project test target or `ctest -R NetworkTaskManager`.

---

## Further reading

- Implementation & layout: `src/modules/NetworkTaskManager/README.md` (internal notes and contributor guidance)
- See `tests/modules/NetworkTaskManager/TestThread.cpp` for a simple download test example.
