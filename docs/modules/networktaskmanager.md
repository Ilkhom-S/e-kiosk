# NetworkTaskManager Module

## Purpose

The NetworkTaskManager module provides HTTP/HTTPS network operations for EKiosk applications. It handles request queuing, retry logic, SSL configuration, and response processing in a thread-safe manner.

## Structure

```
text
src/modules/NetworkTaskManager/
├── CMakeLists.txt          # Build configuration
├── README.md               # Detailed module documentation
├── res/
│   └── *.qrc               # Qt resources
└── src/
    ├── NetworkTaskManager.h # Main manager interface
    ├── NetworkTaskManager.cpp # Implementation
    ├── NetworkTask.h        # Individual task interface
    ├── NetworkTask.cpp      # Task implementation
    ├── SslConfiguration.h   # SSL settings
    └── SslConfiguration.cpp # SSL implementation

include/NetworkTaskManager/
├── NetworkTaskManager.h    # Public headers
├── NetworkTask.h
├── SslConfiguration.h
└── MemoryDataStream.h      # Data stream utilities
```

## Dependencies

- **Qt Core & Network**: For HTTP operations and threading
- **Log module**: For request/response logging
- **SettingsManager module**: For configuration storage

## Platform Compatibility

- **Windows**: Fully supported
- **Linux**: Fully supported
- **macOS**: Fully supported

## Usage

See the module's [README.md](src/modules/NetworkTaskManager/README.md) for detailed usage examples and API documentation.

## CMake Integration

```
cmake
target_link_libraries(MyApp PRIVATE NetworkTaskManager)
```

## Testing

Unit tests are located in `tests/modules/NetworkTaskManager/`. Run with:

```bash
cmake --build build --target test
```

## Migration Notes

Replace direct QNetworkAccessManager usage with this module for consistent network handling across the application.
