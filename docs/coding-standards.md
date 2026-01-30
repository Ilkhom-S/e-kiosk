# Coding Standards

## General

- Use C++14 or later.
- Follow Qt coding conventions for signals/slots, naming, and file structure.
- Each class should have a .cpp/.h pair.
- UI files should use .ui and be edited with Qt Designer.
- **Header Guards:** Use `#pragma once` for all header files to prevent multiple inclusions.

## Formatting

- All C++ code must be formatted using [clang-format](https://clang.llvm.org/docs/ClangFormat.html) with the project-provided [.clang-format](../.clang-format) file at the root of the repository.
- Indent with 4 spaces.
- Use braces for all control blocks.
- Use descriptive variable and function names.
- Do not use tabs (see .clang-format: `UseTab: Never`).
- Run clang-format before committing code. VS Code and most IDEs can be configured to auto-format on save.
- Formatting is enforced in CI and by default in VS Code (see .vscode/settings.json).

**How to format:**

- In VS Code: formatting is automatic on save (see .vscode/settings.json).
- From the command line:

```sh
clang-format -i <file1> <file2> ...
```

**Do not change the .clang-format file without team consensus.**

## Commits

- Use [Conventional Commits](https://www.conventionalcommits.org/).
- Document all breaking or significant changes in the docs.

## CMake Project Structure

- **Always use the EKiosk CMake helpers** (`ek_add_application`, `ek_add_library`, `ek_add_plugin`, `ek_add_test`, etc.) for all new and refactored targets. This ensures consistency, maintainability, and DRY principles across the project.
- See [cmake/README.md](../cmake/README.md) for usage and examples.

## Third-Party Library Versioning

- All Boost dependencies are pinned to version 1.90.0 in vcpkg.json. Do not change Boost versions without updating all dependencies and documentation.

## Documentation

- Update or create relevant docs for any new features, refactors, or breaking changes.
- Link related docs for easy navigation.
- **Module docs convention:** place detailed module documentation in `docs/modules/` (user-facing content). Keep `src/modules/*/README.md` as a concise pointer and include implementation-specific structure and contributor notes there.

## Qt Include Style (Qt5/Qt6 compatibility)

To keep the codebase compatible with both Qt5 and Qt6 and to make dependencies explicit, follow these rules when including Qt headers:

- Prefer module-qualified includes: use `<QtCore/QString>` instead of `<QString>` and `<QtWidgets/QWidget>` instead of `<QWidget>` where possible.
- Wrap Qt headers with the project's warning-suppression wrappers on MSVC:

  #include <Common/QtHeadersBegin.h>
  #include <QtCore/QString>
  #include <Common/QtHeadersEnd.h>

- Include QtCore types explicitly from `QtCore` (examples: `QString`, `QByteArray`, `QVariant`, `QDateTime`, `QList`, `QMap`, `QSet`). This aids future porting to Qt6 and improves IDE indexing.
- When porting legacy code, prefer mechanical refactors (search-and-replace) and run the build and tests after changes.

The project's `.clang-format` and coding guidelines already prefer grouped and sorted includes; run the formatter after changing includes to keep consistency.

## Qt Version Compatibility

All code, CMake, and tests must be written to support both Qt5 and Qt6 where possible.

- Use version-agnostic CMake patterns (e.g., find_package(Qt${QT_VERSION_MAJOR} ...)).
- Prefer Qt APIs and modules available in both versions.
- When Qt version-specific code is required, use CMake or preprocessor checks to handle differences cleanly.

### Platform-Specific Qt Versions

- **Windows 7**: Qt 5.15 LTS (VC toolset 142) - transitional support only
- **Windows 10+ and Linux**: Qt 6.8 LTS
- **macOS**: Qt 6.8 LTS

Use platform detection in CMake to select appropriate Qt versions:

```cmake
if(WIN32)
    if(CMAKE_SYSTEM_VERSION VERSION_GREATER_EQUAL "10.0")
        # Windows 10+ - Qt 6
        find_package(Qt6 REQUIRED)
    else()
        # Windows 7 - Qt 5
        find_package(Qt5 REQUIRED)
    endif()
else()
    # Linux and macOS - Qt 6
    find_package(Qt6 REQUIRED)
endif()
```
