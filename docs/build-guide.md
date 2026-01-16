# Build Guide

This guide explains how to build EKiosk on supported platforms.

## Prerequisites

- **Windows 7**: Qt 5.15 LTS (VC toolset 142) with required modules
- **Windows 10+**: Qt 6.8 LTS with required modules
- **Linux**: Qt 6.8 LTS with required modules
- CMake 3.16+
- Ninja or MSVC (Windows) or GCC/Clang (Linux)

## Platform-Specific Qt Versions

EKiosk automatically selects the appropriate Qt version based on your platform:

- **Windows 7**: Uses Qt 5.15 LTS with VC toolset 142
- **Windows 10+ and Linux**: Uses Qt 6.8 LTS

The CMake configuration will detect your platform and select the correct Qt version automatically.

## Building

1. Configure Qt kits in your IDE or set up environment variables.
2. Configure the project using CMake presets:

   ```sh
   cmake --preset <your-preset>
   ```

3. Build the project:

   ```sh
   cmake --build build/<your-preset>
   ```

4. Run the desired executable from `apps/`.

## Test Mode (Developer/Debug)

EKiosk supports a **test mode** that disables kiosk restrictions (e.g. killing `explorer.exe`, hiding the Windows shell, etc). This is useful for development and debugging.

You can enable test mode in two ways:

- **At runtime:** Launch the application with the `test` argument:

  ```sh
  ekiosk.exe test
  ```

- **At build time (IDE-agnostic):** Configure CMake with the test mode option:

  ```sh
  cmake -DEKIOSK_TEST_MODE=ON --preset <your-preset>
  ```

  This will build the app so that test mode is always enabled, regardless of launch arguments or IDE.

**Recommended:** For IDE-agnostic development, use the CMake option. For quick tests, use the command-line argument.

When test mode is enabled, the app will not kill `explorer.exe` or enforce kiosk shell restrictions.

## EKiosk CMake Helpers (Required)

All new targets (applications, libraries, plugins, tests) **must** use the EKiosk CMake helper functions from the `cmake/` directory. This ensures a consistent, maintainable, and DRY build system for all developers.

- Use `ek_add_application()` for executables
- Use `ek_add_library()` for libraries
- Use `ek_add_plugin()` for plugins
- Use `ek_add_test()` for tests
- Use `ek_install_targets()` and `ek_install_resources()` for install rules
- Use `ek_enable_packaging()` for packaging
- Use `ek_add_translations()` for Qt translations
- Use `ek_enable_static_analysis()` for static analysis (clang-tidy, cppcheck)

**Translations & Qt LinguistTools** ⚠️

- Building translations requires Qt Linguist Tools (`lrelease`/`lupdate`). During configure, CMake runs `find_package(Qt${QT_VERSION_MAJOR} COMPONENTS LinguistTools)`; if not found it falls back to searching for a `lrelease` executable on PATH.
- If you see: `Qt LinguistTools not found. Translation support disabled.` — ensure either:
  - your `CMAKE_PREFIX_PATH` points to a Qt installation that includes LinguistTools (e.g. `-DCMAKE_PREFIX_PATH=C:/Qt/5.15.2/msvc2019_64`), or
  - `lrelease` is on your PATH (Windows: add `C:/Qt/.../bin`), or
  - install the Linguist Tools via the Qt Maintenance Tool.
- You can explicitly disable translation compilation with `-DEK_ENABLE_TRANSLATIONS=OFF`, but translations are required in the repository for correct localization and should be enabled on build machines.

See [cmake/README.md](../cmake/README.md) for detailed usage, rationale, and copy-paste examples for each helper.

**Do not manually define targets or install rules—always use the helpers.**

---

## Code Formatting (clang-format)

All C++ code must be formatted using [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and the project's [.clang-format](../.clang-format) file. Formatting is enforced in CI and by default in VS Code.

- In VS Code: formatting is automatic on save.
- From the command line:

  ```sh
  clang-format -i <file1> <file2> ...
  ```

Do not change the .clang-format file without team consensus.

## Troubleshooting

- Ensure Qt DLLs are in your PATH or next to the executable.
- For MSVC, use the correct preset matching your Qt build.
- See [getting-started.md](getting-started.md) for more help.
