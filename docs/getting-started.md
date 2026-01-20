# Getting Started with EKiosk Development

Welcome to the EKiosk project! This guide will help you set up your development environment and get started quickly.

## Prerequisites

- **Qt 5.15.x (or later)** with Qt Widgets, SerialPort, WebSockets, etc.  
   [Download Qt](https://www.qt.io/download) and install the required modules for your platform.
- **CMake 3.16+**  
   [Download CMake](https://cmake.org/download/)
- **vcpkg** (required for dependency management; the project will not build without it)  
   [Install vcpkg](https://github.com/microsoft/vcpkg#quick-start)
- **Ninja** or **MSVC** (Windows) or **GCC/Clang** (Linux)  
   [Ninja](https://ninja-build.org/), [MSVC](https://visualstudio.microsoft.com/), [GCC](https://gcc.gnu.org/), [Clang](https://clang.llvm.org/)
- **Git**  
   [Download Git](https://git-scm.com/downloads)
- Recommended: Visual Studio Code ([VS Code](https://code.visualstudio.com/)) or Qt Creator ([Qt Creator](https://www.qt.io/product/development-tools))

**Note:** All Boost libraries are pinned to version 1.90.0 in vcpkg.json. Do not change Boost versions unless you update all dependencies and documentation.

## Environment Variables

You can set the following environment variables to customize installation paths (similar to TerminalClient's TC\_\* variables):

| Variable              | Default                       | Description                                                                         |
| --------------------- | ----------------------------- | ----------------------------------------------------------------------------------- |
| `EK_INCLUDES_DIR`     | `${CMAKE_SOURCE_DIR}/include` | Path to public include directory                                                    |
| `EK_BIN_DIR`          | `${CMAKE_INSTALL_BINDIR}`    | Installation directory for executables relative to CMAKE_INSTALL_PREFIX            |
| `EK_LIB_DIR`          | `${CMAKE_INSTALL_LIBDIR}`    | Installation directory for libraries relative to CMAKE_INSTALL_PREFIX              |
| `EK_PLUGIN_DIR`       | `${CMAKE_INSTALL_LIBDIR}/plugins` | Installation directory for plugins relative to CMAKE_INSTALL_PREFIX            |
| `EK_TRANSLATIONS_DIR` | `bin/locale`                  | Installation directory for translation files (.qm) relative to CMAKE_INSTALL_PREFIX |

## Quick Start

1. **Clone the repository:**

```bash
   git clone <repo-url>
   cd ekiosk
```

If the project uses third-party libraries as submodules, initialise them:

```bash
git submodule update --init --recursive
```

For example, this project vendors `SingleApplication` under `thirdparty/SingleApplication`.

The project also includes `thirdparty/CMakeLists.txt`, which will automatically
add any vendored library that contains a `CMakeLists.txt` so its targets become
available to the main build.

1. **Configure Qt Kits:**
   - Use the VS Code Qt Extension or Qt Creator to register your Qt installation.
   - Ensure the correct kit is selected for your platform (MSVC, MinGW, GCC, etc).
2. **Configure the project:**

```bash
cmake --preset <your-preset>
```

- See `CMakePresets.json` for available presets.

1. **Build:**

```bash
cmake --build build/<your-preset>
```

1. **VS Code IntelliSense Setup (Optional):**
   - The project includes `.vscode/c_cpp_properties.json` with pre-configured include paths for Qt and project headers.
   - If IntelliSense still shows include errors, ensure your Qt installation path matches the one in `c_cpp_properties.json` (default: `C:/Qt/5.15.2/msvc2019_64`).
   - For Ninja builds, `compile_commands.json` is automatically generated for better IntelliSense.
   - Reload VS Code window (Ctrl+Shift+P > "Developer: Reload Window") after configuration changes.

---

- Launch the desired app from `apps/` (e.g., `apps/kiosk/`).
- Use your IDE's debugger or run the executable directly.

## vcpkg Caching & Repeated Installs

When using vcpkg in manifest mode (with vcpkg.json), CMake will check dependencies on every configure. Normally, vcpkg restores packages from its local cache (vcpkg_installed/ and your user archives directory) and only installs missing or outdated packages.

**If you see vcpkg installing packages every time:**

- Check that you are not deleting the build/ or vcpkg_installed/ folders between builds unless a full clean is needed.
- Avoid unnecessary changes to vcpkg.json or vcpkg-lock.json, as these trigger reinstalls.
- Use the same triplet (e.g., x64-windows) and toolchain for consistent caching.
- vcpkg caches built packages in `%LOCALAPPDATA%/vcpkg/archives` by default. Do not delete this folder if you want to keep the cache.
- In CI, cache both vcpkg_installed/ and the user archives directory for faster builds.

**Note:** vcpkg will always print a summary of what it is restoring or installing, but if all dependencies are cached, this step is very fast and does not rebuild packages.

For more details, see [vcpkg caching documentation](https://learn.microsoft.com/vcpkg/users/binarycaching/).

## Documentation

- See [architecture.md](architecture.md) for project structure.
- See [build-guide.md](build-guide.md) for detailed build instructions.
- See [coding-standards.md](coding-standards.md) for code style and conventions.
- See [migration-plan.md](migration-plan.md) for ongoing modularization efforts.
- Module docs: Keep detailed module documentation in `docs/modules/` and keep `src/modules/*/README.md` as a short pointer to the canonical docs (this helps searchability and site generation).
- See [Testing Guide](testing.md) for how to run tests locally, add tests, and CI expectations.

## Contributing

- Follow the [conventional commits](https://www.conventionalcommits.org/) standard.
- Update documentation for any breaking or significant changes.
- See [copilot-instructions.md](../.github/copilot-instructions.md) for AI agent guidelines.

## Support

For help, open an issue or contact the maintainers.
