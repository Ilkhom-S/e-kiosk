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
- **EKIniTemplate.cmake**: Generate .ini files from templates with CMake variable substitution (for app/module configuration).

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
include(cmake/EKIniTemplate.cmake)
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

### EKIniTemplate.cmake

**Purpose:** Generate .ini files from templates with CMake variable substitution (for app/module configuration).

**How it works:**

- Provides the `ek_generate_ini_template` macro for generating ini/config files from a template at build time.
- Substitutes CMake variables (e.g. `@WORKING_DIRECTORY@`) in the template with values provided in the macro call.
- Ensures all ini files are generated with up-to-date, build-specific values (e.g. working directory, output paths).

**Usage:**

1. **Include the helper in your root `CMakeLists.txt`:**

   ```cmake
   include(${CMAKE_SOURCE_DIR}/cmake/EKIniTemplate.cmake)
   ```

2. **Prepare a template ini file:**

   Example (`tray.ini.in`):

   ```ini
   [common]
   working_directory = @WORKING_DIRECTORY@
   ```

3. **Call the macro in your app/module `CMakeLists.txt`:**

   ```cmake
   set(WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
   ek_generate_ini_template(tray "${CMAKE_SOURCE_DIR}/runtimes/common/data/tray.ini.in" "${CMAKE_BINARY_DIR}/apps/WatchServiceController" WORKING_DIRECTORY "${WORKING_DIRECTORY}")
   ```

   - The first argument is the output ini name (without extension).
   - The second is the path to the template.
   - The third is the output directory.
   - All following arguments are pairs: `VAR value` (these become `@VAR@` in the template).

**Result:**

- The generated ini file will be at `${CMAKE_BINARY_DIR}/apps/WatchServiceController/tray.ini` with all variables substituted.

**Best practices:**

- Document all template variables in the ini template (in Russian, per project policy).
- Use this macro for all app/module ini/configuration files to ensure consistency and up-to-date values.
- See `runtimes/common/data/tray.ini.in` for a full example.

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
