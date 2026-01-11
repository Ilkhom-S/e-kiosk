# Build Guide

This guide explains how to build EKiosk on supported platforms.

## Prerequisites

- Qt 5.15.x (or later) with required modules
- CMake 3.16+
- Ninja or MSVC (Windows) or GCC/Clang (Linux)

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

See [cmake/README.md](../cmake/README.md) for detailed usage, rationale, and copy-paste examples for each helper.

**Do not manually define targets or install rules—always use the helpers.**

---

## Troubleshooting

- Ensure Qt DLLs are in your PATH or next to the executable.
- For MSVC, use the correct preset matching your Qt build.
- See [getting-started.md](getting-started.md) for more help.
