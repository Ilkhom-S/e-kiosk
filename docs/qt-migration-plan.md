# Qt Version Migration Plan

## Overview

EKiosk will adopt a platform-specific Qt version strategy to maximize compatibility and performance:

- **Windows 7**: Qt 5.15 LTS (VC toolset 142) - transitional support only
- **Windows 10+**: Qt 6.8 LTS
- **Linux**: Qt 6.8 LTS

## Immediate Actions (High Priority)

### 1. Platform Support Updates

- [ ] Drop Windows XP support completely
- [ ] Update platform compatibility documentation
- [ ] Update CMakeLists.txt for platform-specific Qt detection
- [ ] Update CMakePresets.json with platform-specific configurations
- [ ] Set VC toolset to 142 for Windows Qt 5 builds

### 2. Plugin Architecture Changes

- [ ] Create new `WebEngineBackend` plugin for Qt 5.6+ and Qt 6 (WebEngine only)
- [ ] Keep existing `WebKitBackend` plugin for Qt 5.0-5.5 only (deprecated)
- [ ] Add conditional plugin builds in CMake based on Qt version and platform
- [ ] Update QMLBackend for Qt 5/6 compatibility

### 3. QML Compatibility Updates

- [ ] Update all QML imports to `QtQuick 2.15`
- [ ] Add `Qt5Compat.GraphicalEffects 1.15` imports for Qt 6 builds
- [ ] Test QML compatibility across Qt versions
- [ ] Plan migration to versionless imports when fully on Qt 6

### 4. Documentation Updates

- [ ] Update coding standards for Qt version strategy
- [ ] Update copilot instructions with new Qt guidelines
- [ ] Update platform compatibility documentation
- [ ] Update build guide with platform-specific instructions

## Qt Version Compatibility Strategy

### Qt 5 (Windows 7 only)

- Qt 5.15 LTS with VC toolset 142
- Use WebEngineBackend (Qt WebEngine available from 5.6)
- WebKitBackend not available (removed in Qt 5.6)
- Legacy QML imports (will be updated to 2.15)

### Qt 6 (Windows 10+, Linux)

- Qt 6.8 LTS
- WebEngine only (WebEngineBackend plugin)
- Qt5Compat for GraphicalEffects
- Modern Qt 6 APIs where possible

## Migration Timeline

1. **Phase 1**: Update documentation and CMake infrastructure
2. **Phase 2**: Create Qt 6 plugins and update QML
3. **Phase 3**: Test compatibility across platforms
4. **Phase 4**: Deprecate Qt 5 support (when Windows 7 support ends)

## Technical Details

### CMake Changes Required

```cmake
# Platform-specific Qt detection
if(WIN32)
    if(CMAKE_SYSTEM_VERSION VERSION_GREATER_EQUAL "10.0")
        # Windows 10+ - Qt 6
        find_package(QT NAMES Qt6 REQUIRED)
    else()
        # Windows 7 - Qt 5
        find_package(QT NAMES Qt5 REQUIRED)
    endif()
else()
    # Linux - Qt 6
    find_package(QT NAMES Qt6 REQUIRED)
endif()
```

### QML Import Strategy

```qml
import QtQuick 2.15
// Conditional import for Qt 6 compatibility
import Qt5Compat.GraphicalEffects 1.15  // Only for Qt 6
```

### Plugin Conditional Builds

```cmake
# WebKitBackend - Qt 5.0-5.5 only (deprecated)
if(QT_VERSION_MAJOR EQUAL 5 AND QT_VERSION_MINOR LESS 6)
    add_subdirectory(WebKitBackend)
endif()

# WebEngineBackend - Qt 5.6+ and Qt 6
if((QT_VERSION_MAJOR EQUAL 5 AND QT_VERSION_MINOR GREATER_EQUAL 6) OR QT_VERSION_MAJOR EQUAL 6)
    add_subdirectory(WebEngineBackend)
endif()

# WebEngineBackend - Qt 6 only
if(QT_VERSION_MAJOR EQUAL 6)
    add_subdirectory(WebEngineBackend)
endif()
```

## Modularization Migration TODO

This document tracks the migration from a monolithic to a modular architecture.

## Tasks

- [x] Move all code from src/ to apps/kiosk/
- [ ] Refactor shared code into src/ and include/
- [x] Add `BaseApplication` module under `src/modules/common` and port `apps/kiosk` to use it
- [x] Implement DebugUtils module
- [x] Implement Connection module and move tests to tests/modules/
- [x] Implement NetworkTaskManager module and move tests to tests/modules/
- [x] Standardize all adopted modules to use ek_add_library with proper parameters
- [x] Implement LibUSB integration for USB device support in plugins
- [ ] Update CMake to build each app and link shared code
      Notes:
- `SingleApplication` is now vendored as a git submodule under `thirdparty/SingleApplication` to avoid pulling Qt6 via vcpkg. Use `git submodule update --init --recursive` after cloning.- [x] Mirror src/ structure in tests/ (Connection and NetworkTaskManager done)
- [ ] Update documentation for each migration step
- [ ] Review and clean up thirdparty/ dependencies
- [ ] Add CI for modular builds and tests

## Notes

- All breaking or structural changes must be reflected in the docs.
- Keep this list updated as migration progresses.
