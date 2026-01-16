# EKiosk CMake Helper Modules

This directory contains CMake helper modules for the EKiosk project. These modules standardize and simplify the build, test, packaging, and analysis process across all apps and libraries.

## Overview

- **EKLibrary.cmake**: Add and configure shared/static libraries.
- **EKApplication.cmake**: Add and configure applications (executables).
- **EKPlugin.cmake**: Add and configure plugins (Qt or custom).
- **EKTesting.cmake**: Add and configure test targets (Qt Test, CTest, etc.).
- **EKInstall.cmake**: Standardize install rules for apps, libs, and resources.
- **EKPackaging.cmake**: Add packaging rules (CPack, NSIS, etc.).
- **EKTranslation.cmake**: Manage Qt translation files (.ts, .qm).
- **EKStaticAnalysis.cmake**: Integrate static analysis tools (cppcheck, clang-tidy).

---

## Usage

In your `CMakeLists.txt`:

```cmake
include(cmake/EKLibrary.cmake)
include(cmake/EKApplication.cmake)
include(cmake/EKPlugin.cmake)
include(cmake/EKTesting.cmake)
include(cmake/EKInstall.cmake)
include(cmake/EKPackaging.cmake)
include(cmake/EKTranslation.cmake)
include(cmake/EKStaticAnalysis.cmake)
```

---

## Module Details & Examples

### EKLibrary.cmake

**Purpose:** Add a shared or static library with standard settings.

**Example:**

```cmake
ek_add_library(mylib SOURCES src/foo.cpp src/bar.cpp)
```

Folder / IDE grouping

- All `ek_add_*` helpers support an optional `FOLDER` one-value argument that controls the IDE project folder for the generated target (Visual Studio / other IDEs).
- When `FOLDER` is omitted, helpers will compute a sensible default (e.g. `modules/<module-name>` for `ek_add_library` called from `src/modules/<module>`, `plugins/<path>` for plugins, `apps/<app-name>` for applications, and `tests/<path>` for tests).
- You can override the computed value by passing `FOLDER "<your/folder/path>"` in the helper call.

**Why:**

- Consistent library setup (Qt, warnings, install, etc.)
- Reduces boilerplate

---

### EKApplication.cmake

**Purpose:** Add an application (executable) with standard settings.

**Example:**

```cmake
ek_add_application(kiosk SOURCES src/main.cpp)
```

**Why:**

- Consistent app setup (Qt, resources, install)
- Handles platform specifics

---

### EKPlugin.cmake

**Purpose:** Add a plugin (Qt or custom) with correct settings.

**Example:**

```cmake
ek_add_plugin(myplugin SOURCES src/plugin.cpp)
```

**Why:**

- Ensures correct plugin type, install location, Qt macros

---

### EKTesting.cmake

**Purpose:** Add test targets (unit/integration) using Qt Test or CTest.

**Example:**

```cmake
ek_add_test(mytest SOURCES tests/mytest.cpp)
```

**Why:**

- Standardizes test setup, enables CI integration

---

### EKInstall.cmake

**Purpose:** Standardize install rules for all targets and resources.

**Example:**

```cmake
ek_install_targets(kiosk mylib)
ek_install_resources(assets/ styles/)
```

**Why:**

- Consistent install layout, easy packaging

---

### EKPackaging.cmake

**Purpose:** Add packaging rules (CPack, NSIS, etc.).

**Example:**

```cmake
ek_enable_packaging()
```

**Why:**

- One-command packaging for all platforms

---

### EKTranslation.cmake

**Purpose:** Manage Qt translation files (.ts, .qm).

**Example:**

```cmake
ek_add_translations(${CMAKE_SOURCE_DIR}/translations/*.ts)
```

**Why:**

- Automates lrelease, ensures all languages are built

---

### EKStaticAnalysis.cmake

**Purpose:** Integrate static analysis tools (cppcheck, clang-tidy) for code quality.

**Example:**

```cmake
ek_enable_static_analysis(kiosk CPPCHECK CLANG_TIDY)
```

**Why:**

- Enforces code standards, catches bugs early

---

## Best Practices

- Always use these helpers for new targets.
- Keep helper modules up to date with project standards.
- See each module for advanced options and customization.

---

## See Also

- [Build Guide](../docs/build-guide.md)
- [Coding Standards](../docs/coding-standards.md)
- [Migration TODO](../docs/migration-todo.md)
- [Architecture](../docs/architecture.md)
