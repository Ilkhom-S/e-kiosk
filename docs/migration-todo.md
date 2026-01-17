# Modularization Migration TODO

This document tracks the migration from a monolithic to a modular architecture.

## Tasks

- [x] Move all code from src/ to apps/kiosk/
- [ ] Refactor shared code into src/ and include/
- [x] Add `BaseApplication` module under `src/modules/common` and port `apps/kiosk` to use it
- [x] Implement DebugUtils module
- [x] Modernize DebugUtils to use Boost.Stacktrace (cross-platform stack tracing)
- [x] Implement Connection module and move tests to tests/modules/
- [x] Implement NetworkTaskManager module and move tests to tests/modules/
- [x] Standardize all adopted modules to use ek_add_library with proper parameters
- [x] Implement LibUSB integration for USB device support in plugins
- [x] Fix Ad plugin tests: move to tests/plugins/Ad/, fix Qt cleanup crash, enhance coverage
- [x] Implement FR (Fiscal Register) deprecation Phase 1 - exclude from build system
- [ ] Update CMake to build each app and link shared code
      Notes:
- `SingleApplication` is now vendored as a git submodule under `thirdparty/SingleApplication` to avoid pulling Qt6 via vcpkg. Use `git submodule update --init --recursive` after cloning.- [x] Mirror src/ structure in tests/ (Connection and NetworkTaskManager done)
- [ ] Update documentation for each migration step
- [ ] Review and clean up thirdparty/ dependencies
- [ ] Add CI for modular builds and tests
- [x] Implement platform-specific Qt version strategy (Qt 5.15 LTS for Windows 7, Qt 6.8 LTS for Windows 10+/Linux)
- [x] Update all documentation for Qt version strategy (platform-compatibility.md, coding-standards.md, copilot-instructions.md, build-guide.md)
- [x] Fix interface header AUTOMOC processing for PPSDK and Connection modules
- [x] Refactor QMLBackend plugin: merge `PluginFactoryDefinition._` into `QMLBackendFactory._`, update CMake to explicit sources, add unit tests
- [x] Refactor NativeBackend plugin: merge `PluginFactoryDefinition._` into `NativeBackendFactory._`, update CMake to explicit sources, add unit tests
- [x] Refactor WebEngineBackend plugin: merge `PluginFactoryDefinition._` into `WebEngineBackendFactory._`, update CMake to explicit sources, add unit tests

## Notes

- All breaking or structural changes must be reflected in the docs.
- Keep this list updated as migration progresses.
