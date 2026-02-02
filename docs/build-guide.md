# Build Guide

This guide explains how to build EKiosk on supported platforms.

## Prerequisites

- **Windows 7**: Qt 5.15 LTS (VC toolset 142) with required modules
- **Windows 10+**: Qt 6.8 LTS with required modules
- **Linux**: Qt 6.8 LTS with required modules
- **macOS**: Qt 6.8 LTS with required modules
- CMake 3.16+
- Ninja or MSVC (Windows) or GCC/Clang (Linux/macOS)

## Platform-Specific Qt Versions

EKiosk automatically selects the appropriate Qt version based on your platform:

- **Windows 7**: Uses Qt 5.15 LTS with VC toolset 142
- **Windows 10+ and Linux**: Uses Qt 6.8 LTS
- **macOS**: Uses Qt 6.8 LTS

The CMake configuration will detect your platform and select the correct Qt version automatically.

## Qt Environment Variables

Before configuring the project, set the appropriate Qt path environment variables based on your platform and Qt version:

- **Windows Qt 5 x64 MSVC**: `QT5_X64_PATH=C:/Qt/5.15.2/msvc2019_64`
- **Windows Qt 6 x64 MSVC**: `QT6_X64_PATH=C:/Qt/6.8.0/msvc2019_64`
- **Windows Qt 5 x86 MSVC**: `QT5_X86_PATH=C:/Qt/5.15.2/msvc2019`
- **Windows Qt 6 x86 MSVC**: `QT6_X86_PATH=C:/Qt/6.8.0/msvc2019`
- **Linux Qt 6 x64**: `LINUX_QT_PATH=/opt/Qt/6.8.0/gcc_64`
- **macOS Qt 6 x64**: `MACOS_QT_PATH=/opt/Qt/6.8.0/macos`

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

## macOS Build and Deployment

EKiosk builds create proper macOS app bundles for GUI applications, with automatic ad-hoc code signing for development.

### Building on macOS

1. Ensure Qt 6.8 LTS is installed (e.g., via Homebrew or official installer).
2. Set `CMAKE_PREFIX_PATH` to your Qt installation:

   ```sh
   export CMAKE_PREFIX_PATH="/opt/Qt/6.8.0/macos"
   ```

3. Configure and build as usual:

   ```sh
   cmake --preset <your-preset>
   cmake --build build/<your-preset>
   ```

GUI applications will be built as `.app` bundles in `build/<preset>/bin/`.

### Running Applications

For bundled apps, run the executable inside the bundle:

```sh
./build/<preset>/bin/tray.app/Contents/MacOS/tray
```

For debugging, use LLDB or Xcode:

```sh
lldb ./build/<preset>/bin/tray.app/Contents/MacOS/tray
```

### Code Signing

- **Development**: Automatic ad-hoc signing ensures apps run without code signing validation errors during debugging.
- **Production**: For distribution, sign with a proper Apple Developer certificate:

  ```sh
  codesign --force --deep --sign "Developer ID Application: Your Name" your-app.app
  ```

**Troubleshooting Code Signing Issues:**

If you encounter `__cxa_throw` exceptions from the Security framework during debugging:

1. **Disable Qt code signing validation**: Set the environment variable `QT_MAC_DISABLE_CODE_SIGNING_CHECK=1` in your application code before creating QApplication:

   ```cpp
   qputenv("QT_MAC_DISABLE_CODE_SIGNING_CHECK", "1");
   ```

2. **Ensure ad-hoc signing is enabled**: The CMake build system automatically applies ad-hoc signing (`codesign --force --deep --sign -`) to all macOS applications and plugins.

3. **Check for proper Qt attributes**: Applications should set `QApplication::setAttribute(Qt::AA_DisableSessionManager)` before creating the QApplication instance.

4. **Verify bundle structure**: GUI applications must be built as `MACOSX_BUNDLE` targets.

5. **For production builds**: Use a proper Apple Developer certificate instead of ad-hoc signing.

### Debugging macOS Apps

- Use LLDB for debugging: `lldb path/to/executable`
- Common issues:
  - **Code signing validation errors**: Ensure ad-hoc signing is applied and Qt attributes are set properly (see Code Signing section above)
  - Bundle structure: GUI apps must be built as MACOSX_BUNDLE
  - Entitlements: Add plist for special permissions (e.g., accessibility)

### Deployment

- Bundles include all necessary Qt frameworks via `macdeployqt`
- For distribution, notarize with Apple:

  ```sh
  xcrun altool --notarize-app --primary-bundle-id "com.yourcompany.ekiosk" --username "your-apple-id" --password "app-specific-password" --file your-app.zip
  ```

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

- **VS Code debugging:** If you use VS Code to debug Qt applications, set the `QT_BIN_PATH` environment variable to your Qt `bin` directory (e.g. `D:/Qt/5.15.2/msvc2019_64/bin`) before launching VS Code. This allows the debugger to find the required Qt DLLs automatically, without hardcoding paths in `.vscode/launch.json`. See [docs/vs-code-debug-qt-path.md](vs-code-debug-qt-path.md) for details.

### INI Template Generation (ek_generate_ini_template)

For all application and module configuration files (.ini), EKiosk uses a CMake macro to generate ini files from templates with build-time variable substitution. This ensures all config files are up-to-date and documented in Russian.

**How to use:**

1. Prepare a template ini file (e.g. `tray.ini.in`) with CMake variables like `@WORKING_DIRECTORY@`.
2. In your app/module `CMakeLists.txt`, call:

```cmake
set(WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
ek_generate_ini_template(tray "${CMAKE_SOURCE_DIR}/runtimes/common/data/tray.ini.in" "${CMAKE_BINARY_DIR}/apps/WatchServiceController" WORKING_DIRECTORY "${WORKING_DIRECTORY}")
```

- The first argument is the output ini name (no extension).
- The second is the template path.
- The third is the output directory.
- All following arguments are pairs: `VAR value` (these become `@VAR@` in the template).

**Result:** The generated ini file will be in the build output directory, with all variables replaced by their build-time values.

- Always document all variables in the template in Russian.
- See `cmake/README.md` for full details and examples.

## Icon Generation

EKiosk uses a Python script to generate icon files from SVG templates for cross-platform compatibility.

### Generating Icons

Run the icon generation script to create PNG, ICO, and ICNS files from SVG sources:

```sh
python3 scripts/generate_icons.py --apps <app-name> [--force]
```

- `--apps`: Specify which app's icons to generate (e.g., `WatchServiceController`)
- `--force`: Regenerate all icons even if they exist

### Template Icons for macOS

For macOS light/dark theme support:

1. **SVG Templates**: Name SVG files with "Template" suffix (e.g., `iconTemplate.svg`)
2. **Qt Integration**: Use `QIcon(":/icons/iconTemplate.png")` and call `icon.setIsMask(true)`
3. **Automatic Theme Adaptation**: macOS automatically inverts template icons based on system theme
4. **Black Fill**: Use solid black (#000000) fill in SVG templates for best results

**Important Limitations:**

- Template icons are monochrome and invert as a whole
- Colored elements (like status dots) will not adapt properly
- Context menu disabled states may not follow template behavior
- For colored status indicators, use separate non-template icons

The script automatically generates all required icon sizes and formats from the SVG templates.
