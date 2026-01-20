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

## Qt Environment Variables

Before configuring the project, set the appropriate Qt path environment variables based on your platform and Qt version:

- **Windows Qt 5 x64 MSVC**: `QT5_X64_PATH=C:/Qt/5.15.2/msvc2019_64`
- **Windows Qt 6 x64 MSVC**: `QT6_X64_PATH=C:/Qt/6.8.0/msvc2019_64`
- **Windows Qt 5 x86 MSVC**: `QT5_X86_PATH=C:/Qt/5.15.2/msvc2019`
- **Windows Qt 6 x86 MSVC**: `QT6_X86_PATH=C:/Qt/6.8.0/msvc2019`
- **Linux Qt 6 x64**: `LINUX_QT_PATH=/opt/Qt/6.8.0/gcc_64`

Adjust the paths to match your actual Qt installation directories. These variables are used by the CMake presets to automatically set `CMAKE_PREFIX_PATH`.

For permanent setup, add them to your system environment variables or include them in your development environment configuration.

## Building

1. Configure Qt kits in your IDE or set up environment variables.
2. Configure the project using CMake presets:

   ```sh
   cmake --preset <your-preset>
   ```

   Note: You can override the project's public include directory (useful when using a separate header tree or during porting) by setting the `EK_INCLUDES_DIR` environment variable or passing `-DEK_INCLUDES_DIR` to CMake. Examples:
   - PowerShell (temporary for the session):

     ```powershell
     $env:EK_INCLUDES_DIR = 'C:\path\to\includes'
     cmake --preset <your-preset>
     ```

   - CMake configure-time override:

     ```sh
     cmake -DEK_INCLUDES_DIR=/path/to/includes --preset <your-preset>
     ```

   You can also override the translations installation directory by setting the `EK_TRANSLATIONS_DIR` environment variable. This controls where translation files (.qm) are installed relative to CMAKE_INSTALL_PREFIX. Default is `bin/locale`.

   Additional environment variables for installation paths:
   - `EK_BIN_DIR`: Override executable installation directory (default: `${CMAKE_INSTALL_BINDIR}`)
   - `EK_LIB_DIR`: Override library installation directory (default: `${CMAKE_INSTALL_LIBDIR}`)
   - `EK_PLUGIN_DIR`: Override plugin installation directory (default: `${CMAKE_INSTALL_LIBDIR}/plugins`)

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

Folder / IDE grouping

- The `ek_add_*` helpers accept an optional `FOLDER` argument to explicitly control the IDE project folder for the generated target (Visual Studio, CLion, etc.).
- If `FOLDER` is omitted, the helper will compute a sensible default (for example: `modules/<module-name>`, `plugins/<path>`, `apps/<app-name>`, `tests/<path>`).
- To override the default, pass `FOLDER "<folder/path>"` in the helper call, e.g. `ek_add_plugin(myplugin FOLDER "plugins/Payments/Humo" SOURCES ...)`.

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
