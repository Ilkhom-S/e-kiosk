# Coding Standards

## C++ Guidelines

- **Use Qt Types:** Prefer QString, QList, QMap over STL equivalents.
- **Smart Pointers:** Use QScopedPointer, QSharedPointer, or std::unique_ptr as appropriate.
- **Naming Conventions:**
  - Classes: PascalCase (e.g., DeviceManager)
  - Methods: camelCase (e.g., initializeDevice)
  - Member variables: m_camelCase (e.g., m_deviceList)
  - Constants: UPPER_SNAKE_CASE or kPascalCase
  - Interfaces: Prefix with I (e.g., IDevice, ICryptEngine)
- **Header Guards:** Use #pragma once
- **Include Order:**
  1. Corresponding header
     #include "MyClass.h"
  2. Project headers
     #include "Common/QtHeadersBegin.h"
     #include <QtCore/QString>
     #include "Common/QtHeadersEnd.h"
  3. Third-party headers
     #include <boost/optional.hpp>
  4. Standard library
     #include <vector>

## Qt-Specific Guidelines

- **Qt5/Qt6 Compatibility:**
  - Use version macros for compatibility:
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Qt6 code
    #else
    // Qt5 code
    #endif
- **Signal/Slot Syntax:** Prefer new syntax:
  connect(sender, &Sender::signal, receiver, &Receiver::slot);
- **MOC Requirements:** Classes with signals/slots need Q_OBJECT macro.

## CMake Guidelines

- **Use EKiosk CMake Helpers:** Always use the helper functions from cmake/ for all new targets and when refactoring existing CMakeLists.txt files. These include:

  - ek_add_library() from EKLibrary.cmake
  - ek_add_application() from EKApplication.cmake
  - ek_add_plugin() from EKPlugin.cmake
  - ek_add_test() from EKTesting.cmake
  - ek_install_targets(), ek_install_resources() from EKInstall.cmake
  - ek_enable_packaging() from EKPackaging.cmake
  - ek_add_translations() from EKTranslation.cmake
  - ek_enable_static_analysis() from EKStaticAnalysis.cmake

  See cmake/README.md for detailed usage, rationale, and examples for each helper.

**Why:**

- Ensures consistent build, test, install, and packaging logic across all apps and modules
- Simplifies CMakeLists.txt files and reduces boilerplate
- Enforces code quality and static analysis standards
- Makes it easy for new contributors to follow project conventions

**How to Refactor:**

- When updating or creating CMakeLists.txt, replace manual target definitions with the appropriate ek\_\* helper function(s).
- Always include the relevant cmake/\*.cmake modules at the top of your CMakeLists.txt.
- Refer to cmake/README.md for copy-paste examples and advanced options.
- If a use case is not covered, extend the helper or document the exception in the code and docs.
- **Qt Version Agnostic:** Use Qt${QT_VERSION_MAJOR}::Module syntax.
- **Platform Checks:** Use if(WIN32), if(UNIX AND NOT APPLE), if(APPLE) for platform-specific code.

# Qt Version Compatibility

All code, CMake, and tests must be written to support both Qt5 and Qt6 where possible.

- Use version-agnostic CMake patterns (e.g., find_package(Qt${QT_VERSION_MAJOR} ...)).
- Prefer Qt APIs and modules available in both versions.
- When Qt version-specific code is required, use CMake or preprocessor checks to handle differences cleanly.

## Conventional Commits & Scopes

All commits must follow the [Conventional Commits](https://www.conventionalcommits.org/) standard.

**Recommended scopes for this project:**

- apps/kiosk, apps/updater, apps/guard
- src, include, thirdparty, tests, docs, cmake
- devices, modules, ui, connection, db, other, updater
- build, ci, config, migration

**Examples:**

- feat(apps/kiosk): add new payment module
- fix(devices): resolve printer timeout issue
- docs(getting-started): update setup instructions

Use clear, descriptive scopes to indicate which part of the project is affected.

# Auto-Commit Policy

For all changes that do not require user review (e.g., documentation updates, test scaffolding, non-breaking code changes, or changes that are successfully tested and build without errors), Copilot should auto-commit with a clear, conventional commit message. Only request user confirmation for breaking changes, ambiguous refactors, or when tests/builds fail.

# EKiosk Qt C++ Project - AI Coding Agent Instructions

## Folder Structure (2026 Modular Redesign)

- **apps/**: Contains all executable applications (e.g., kiosk, updater, guard). Each app has its own folder.
  - Each app (e.g., `kiosk`) has its own `src/` for code, and a `CMakeLists.txt` at the app root.
  - Platform/build folders (e.g., `msvc/`) can also be placed at the app root.
- **src/**: Shared code and libraries, to be refactored from apps/kiosk/src as modularization proceeds.
- **include/**: Public headers for use across modules and apps.
- **thirdparty/**: External/third-party libraries.
- **tests/**: Unit and integration tests for all apps and modules.
  - Mirrors the structure of `src/` and `apps/` for test coverage.
  - Each app/module can have its own test subfolder (e.g., `tests/kiosk/`, `tests/Updater/`).
  - Use CMake to add test targets for each app/module; top-level `tests/CMakeLists.txt` adds all test subdirectories.
  - Use Qt Test or another C++ testing framework.
- **resources/**: Images, sounds, styles, etc.
- **docs/**: Project documentation.
- **cmake/**: Custom CMake modules.

## Documentation Requirements

- Any change that breaks, restructures, or introduces new features **must** be reflected in the relevant docs (architecture, build-guide, coding-standards, migration-todo, etc).
- Keep documentation up to date and link between related docs for easy navigation.

## Migration Plan

- Move all code from `src/` to `apps/kiosk/` as a first step.
- Refactor shared code into `src/` and `include/` as modularization proceeds.
- Update CMake to build each app and link shared code.
- Track migration progress in [docs/migration-todo.md](../docs/migration-todo.md).

## Best Practices

- Keep apps, shared code, and third-party libraries clearly separated.
- Use CMake targets for each app and library.
- Mirror src/ structure in tests/ for coverage.
- Follow conventional commits and update docs for all major changes.

## References

- [Getting Started](../docs/getting-started.md)
- [Build Guide](../docs/build-guide.md)
- [Coding Standards](../docs/coding-standards.md)
- [Platform Compatibility](../docs/platform-compatibility.md)
- [Multilanguage Support](../docs/multilanguage.md)
- [Migration Plan & TODO](../docs/migration-todo.md)

---

_For any major change, update the docs and migration-todo list!_
