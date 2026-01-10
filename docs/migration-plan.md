# Qt 4 to Qt 5 Migration Plan

## Overview

This plan outlines the step-by-step migration from Qt 4 to Qt 5 and adoption of CMake build system for the EKiosk project. Qt 5 introduces significant changes, including module separation and API updates. CMake provides better cross-platform build management.

## Prerequisites

- Backup current code (branch `qt4-fallback` created).
- Install Qt 5 development environment.
- Ensure C++11 support in compiler.

## Step-by-Step Plan

### 1. Update Project File (.pro)

- [x] Add `widgets` module: `QT += core gui widgets`
- [x] Add minimum version: `requires qt 5.0`
- [x] Remove conditional `greaterThan(QT_MAJOR_VERSION, 4): QT += widgets` if not needed.

### 2. Review Dependencies

- [ ] Check for Qt 4 deprecated modules (e.g., `QtWebKit` → `QtWebEngine` if applicable).
- [ ] Update any third-party libs for Qt 5 compatibility.
- [ ] Verify Windows libs (rasapi32, etc.) are still compatible.

### 3. Code Review for Deprecated APIs

- [ ] Search for Qt 4 specific APIs:
  - `QApplication::setGraphicsSystem()` (removed in Qt 5).
  - `Q_WS_*` macros (replaced with `Q_OS_*`).
  - `QDesktopServices::storageLocation()` (changed to `standardLocations()`).
- [ ] Update signal/slot connections if using old syntax (current code uses macros, which are compatible).
- [ ] Check for `QVariant::toString()` changes or other type conversions.

### 4. Migrate Build System to CMake

- [ ] Create `CMakeLists.txt` with Qt 5 support using `find_package(Qt5 REQUIRED COMPONENTS Core Widgets ...)`.
- [ ] Configure source files, include directories, and libraries (rasapi32, etc.).
- [ ] Set up executable target with `add_executable`.
- [ ] Handle UI files with `qt5_wrap_ui`, resources with `qt5_add_resources`.
- [ ] Test CMake build on target platforms.
- [ ] Update build instructions in README.

### 5. Update Build System

- [ ] Regenerate Makefiles with qmake for Qt 5 (if keeping qmake as fallback).
- [ ] Update any custom build scripts.
- [ ] Test compilation on target platforms (Windows, Linux, Mac).

### 6. Test Application

- [ ] Build and run the application.
- [ ] Verify UI rendering (widgets module).
- [ ] Test device communications (serial ports).
- [ ] Check network functionality (WebSockets, JSON requests).
- [ ] Validate logging and error handling.

### 7. Address Any Issues

- [ ] Fix compilation errors related to API changes.
- [ ] Update code for new Qt 5 features if beneficial (e.g., new signal/slot syntax).
- [ ] Ensure single-instance checking works (`QSharedMemory`, `QSystemSemaphore`).

### 8. Documentation and Cleanup

- [ ] Update README and docs for Qt 5 requirements.
- [ ] Remove any Qt 4 compatibility code.
- [ ] Commit changes with clear messages.

### 9. Final Validation

- [ ] Run full test suite (if exists).
- [ ] Deploy to test environment.
- [ ] Monitor for runtime issues.

## Risks and Rollback

- If issues arise, switch to `qt4-fallback` branch.
- Qt 5 may have performance or compatibility differences with hardware.

## Timeline

- Estimated: 1-2 weeks depending on complexity found during review.
