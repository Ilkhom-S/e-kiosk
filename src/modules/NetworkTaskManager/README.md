# NetworkTaskManager

This folder contains the module implementation. The full, canonical documentation has been moved to the central docs site:

- See: `../../../docs/modules/networktaskmanager.md`

## Structure (implementation)

```text
src/modules/NetworkTaskManager/
├── CMakeLists.txt                  # Module build configuration
├── include/NetworkTaskManager/     # Public headers (installed targets)
│   ├── NetworkTaskManager.h
│   └── ...
├── src/
│   ├── NetworkTaskManager.h/.cpp   # Manager interface & implementation
│   ├── NetworkTask.h/.cpp          # Network task implementation
│   ├── SslConfiguration.h/.cpp     # SSL settings & helpers
│   └── ...                         # internal helpers, tests, etc.
├── res/
│   └── *.qrc                       # Qt resources
└── README.md                       # This file (short pointer to canonical docs)
```

**Contributor notes:**

- Keep high-level documentation, examples, and usage in `docs/modules/networktaskmanager.md`.
- Use this README for implementation notes, file layout, build or testing hints and internal design details.
- For API docs, examples, and user-facing information, see the canonical docs.

## Modes & flags (brief)

This module supports per-task flags that change execution behavior. Document user-facing usage in `docs/modules/networktaskmanager.md`, but keep a short summary here for contributors:

- **BlockingMode**: Run a task synchronously inside the manager thread; consumer can use `waitForFinished()` to block until completion.
- **Continue**: Resume partial downloads (used by `FileDownloadTask`); the module will append data to existing file streams.
- **IgnoreErrors**: Attempt to continue downloads on transient errors until timeout is reached.

When adding new flags or modes, update both the docs page and this short reference so contributors can quickly see implementation implications.

## API summary (short)

- **Primary classes:** `NetworkTaskManager`, `NetworkTask`, `FileDownloadTask`, `DataStream` (File/Mem), `IVerifier` and verifiers (`Md5Verifier`, `Sha256Verifier`).
- **Common signals:** `NetworkTask::onProgress(qint64 current, qint64 total)`, `NetworkTask::onComplete()`, `NetworkTaskManager::networkTaskStatus(bool failure)`.
- **Where to look for full signatures:** `include/NetworkTaskManager/` and `src/modules/NetworkTaskManager/src/`.

Keep this summary updated when APIs change so reviewers can quickly spot relevant changes.
