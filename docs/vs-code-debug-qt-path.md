# VS Code Debugging: Qt DLL Path Setup

To debug EKiosk or any Qt-based app in VS Code without hardcoding the Qt DLL path, use the following environment variable approach:

## Step-by-step

1. **Set the environment variable before launching VS Code:**
   - Open a terminal and run:

     ```bash
     set QT_BIN_PATH=D:/Qt/5.15.2/msvc2019_64/bin
     ```

   - Or add `QT_BIN_PATH` to your Windows user/system environment variables for permanent use.

2. **launch.json configuration:**
   - All debug configurations in `.vscode/launch.json` should use:

     ```json
     "environment": [
     {
     "name": "PATH",
     "value": "${env:QT_BIN_PATH};${workspaceFolder}/vcpkg_installed/x64-windows/bin;${env:PATH}",
     "language": "Python"
     ]
     ```

   - This ensures the debugger finds the correct Qt DLLs for your build, regardless of where Qt is installed.

3. **CMakePresets.json**
   - CMake uses `$env{QT5_X64_PATH}` or similar for configuration. This is separate from the debugger and must also be set.

## Why?

- This avoids hardcoding paths in your project files.
- Makes your setup portable across machines and developer environments.
- Matches the CMake environment variable strategy for builds.

## Troubleshooting

- If you see error 0xc0000135 (missing DLL), check that `QT_BIN_PATH` is set and points to the correct Qt `bin` directory.
- You can verify the environment variable in a terminal with:

  ```bash
  echo %QT_BIN_PATH%
  ```

---

**Reference:**

- `.vscode/launch.json` (debugger config)
- `CMakePresets.json` (build config)
- [docs/build-guide.md](../docs/build-guide.md)
