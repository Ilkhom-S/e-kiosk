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

## Troubleshooting

- Ensure Qt DLLs are in your PATH or next to the executable.
- For MSVC, use the correct preset matching your Qt build.
- See [getting-started.md](getting-started.md) for more help.
