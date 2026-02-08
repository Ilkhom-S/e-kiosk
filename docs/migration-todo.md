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
- [x] Complete header refactoring: migrate all major redirect headers from src/modules/ to include/ with proper deprecation notes (FR protocols, CashAcceptor protocols, SparkFR.h, data types, Scanners, IOPorts, Watchdogs, CashAcceptors, DispenserBase, Printers collections, CryptEngine, CreatorReader, and remaining individual headers completed)
- [x] Complete Hardware module header migration: migrate HardwareCommon, CashAcceptors, and other hardware protocol headers to resolve plugin linking issues
- [x] Implement SingleApplication for single-instance enforcement in WatchServiceController
- [x] Fix service_menu plugin linking: resolve IConnection, PaymentProcessor classes, and IDeviceTest Qt meta-object undefined symbols by adding macOS implementations, updating dependencies to include PPSDK, and generating moc_IDeviceTest.cpp
- [x] Refactor Printer plugin: add CoreVersion.rc, DriversSDK, conditional AxContainer, translations; fix template instantiation and class definitions
      Notes:
- `SingleApplication` is now vendored as a git submodule under `thirdparty/SingleApplication` to avoid pulling Qt6 via vcpkg. Use `git submodule update --init --recursive` after cloning.
- [x] Mirror src/ structure in tests/ (Connection and NetworkTaskManager done)
- [ ] Update documentation for each migration step
- [ ] Review and clean up thirdparty/ dependencies
- [ ] Add CI for modular builds and tests
- [x] Implement platform-specific Qt version strategy (Qt 5.15 LTS for Windows 7, Qt 6.8 LTS for Windows 10+/Linux)
- [x] Implement macOS support with Qt 6.8 LTS, app bundles, and automatic code signing
- [x] Update all documentation for Qt version strategy (platform-compatibility.md, coding-standards.md, copilot-instructions.md, build-guide.md)
- [x] Fix interface header AUTOMOC processing for PPSDK and Connection modules
- [x] Refactor QMLBackend plugin: merge `PluginFactoryDefinition._` into `QMLBackendFactory._`, update CMake to explicit sources, add unit tests
- [x] Refactor NativeBackend plugin: merge `PluginFactoryDefinition._` into `NativeBackendFactory._`, update CMake to explicit sources, add unit tests
- [x] Refactor WebEngineBackend plugin: merge `PluginFactoryDefinition._` into `WebEngineBackendFactory._`, update CMake to explicit sources, add unit tests
- [x] Update TemplatePlugin to use C++ static metadata instead of JSON files
- [x] Complete Migrator3000 scenario plugin modernization: remove Qt plugin dependencies, add PPSDK and QtScript/QtQml dependencies, fix AUTOMOC processing
  - Note: Migrator3000 is now deprecated and will be removed in a future release
- [x] Fix Qt 6 deprecation warnings for QScopedPointer::take() usage throughout the codebase, replacing with std::unique_ptr and release() method
- [x] Fix AUTOMOC processing issues for Qt classes with Q_OBJECT macros by adding public headers to CMake SOURCES (GraphicsEngine, DeviceManager, ScenarioEngine, UpdateEngine)
- [x] Fix compiler warning for unhandled enumeration values 'EMoney' and 'BankCard' in PaymentDatabaseUtils.cpp switch statement
- [x] Fix Boost.Bind deprecation warning by updating includes to use modern <boost/bind/bind.hpp> + using namespace boost::placeholders
- [ ] Migrate remaining QRegExp usages to QRegularExpression for Qt 6 compatibility

## Version Pinning

- All Boost dependencies are pinned to version 1.90.0 in vcpkg.json for reproducible builds. Update this note and vcpkg.json if the version changes.

## Notes

- All breaking or structural changes must be reflected in the docs.
- Keep this list updated as migration progresses.
