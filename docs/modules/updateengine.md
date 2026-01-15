# UpdateEngine Module

## Purpose

The UpdateEngine module provides comprehensive update management functionality for EKiosk applications, including downloading, verifying, and installing software updates from remote servers.

---

## Quick start 🔧

```cpp
#include "UpdateEngine/UpdateEngine.h"

UpdateEngine* updater = new UpdateEngine();
updater->setUpdateUrl(QUrl("https://updates.example.com/manifest.xml"));
updater->checkForUpdates();

// Connect to signals
connect(updater, &UpdateEngine::updateAvailable, this, &MyClass::onUpdateAvailable);
connect(updater, &UpdateEngine::updateProgress, this, &MyClass::onUpdateProgress);
```

---

## Features

- **Update checking**: Automatic or manual checking for available updates
- **Manifest parsing**: XML-based update manifest parsing
- **Download management**: Resumable downloads with progress tracking
- **Integrity verification**: MD5/SHA checksum verification of downloaded files
- **Installation**: Automated installation of updates with rollback capability
- **Version management**: Version comparison and dependency checking
- **Network integration**: Uses NetworkTaskManager for all network operations
- **Progress reporting**: Detailed progress signals for UI integration
- **Error handling**: Comprehensive error reporting and recovery
- **Qt6 compatibility**: Handles XmlPatterns deprecation in Qt6

---

## Platform support

| Platform | Status  | Notes                                          |
| -------- | ------- | ---------------------------------------------- |
| Windows  | ✅ Full | Full update support with installer integration |
| Linux    | ✅ Full | Full update support with package management    |
| macOS    | ✅ Full | Full update support with installer integration |

---

## Configuration

### Update Manifest Format

```xml
<updates>
  <update version="1.2.3">
    <file name="app.exe" url="https://..." md5="..." size="..."/>
    <file name="lib.dll" url="https://..." sha256="..." size="..."/>
  </update>
</updates>
```

### CMake Configuration

```cmake
if(QT_VERSION_MAJOR EQUAL 5)
    ek_add_library(UpdateEngine
        QT_MODULES Core Network Xml XmlPatterns
        DEPENDS NetworkTaskManager
    )
else()
    ek_add_library(UpdateEngine
        QT_MODULES Core Network Xml
        DEPENDS NetworkTaskManager
        COMPILE_DEFINITIONS QT6_NO_XMLPATTERNS
    )
endif()
```

---

## Usage / API highlights

### Key Classes

- `UpdateEngine`: Main update management class
- `UpdateManifest`: Update manifest parser
- `UpdateTask`: Individual update task handler
- `UpdateInstaller`: Platform-specific installer

### Common Operations

```cpp
// Check for updates
updater->checkForUpdates();

// Download and install
updater->downloadUpdate(updateInfo);
updater->installUpdate(updateInfo);

// Progress monitoring
connect(updater, &UpdateEngine::updateProgress,
        this, [](int percent, const QString& status) {
    // Update UI
});
```

---

## Integration

### Dependencies

- NetworkTaskManager: For all network operations
- Qt Xml/XmlPatterns: For manifest parsing
- Boost: For various utilities

### Build System

Uses EKiosk CMake helpers with Qt version-specific handling for XmlPatterns.

---

## Testing

Unit tests cover:

- Manifest parsing
- Download verification
- Installation process
- Error handling scenarios

Run tests with: `ctest -R UpdateEngine`

---

## Migration notes

- Ported from TerminalClient project
- Updated to use EKiosk build system and NetworkTaskManager
- Qt6 compatibility with XmlPatterns handling
- Enhanced error reporting and progress tracking

---

## Further reading

- Implementation details: `src/modules/UpdateEngine/README.md`
- API documentation: `include/UpdateEngine/` headers</content>
  <parameter name="filePath">c:\Projects\Humo\Kiosk\ekiosk\docs\modules\updateengine.md
