# Multi-Platform Debug Configurations

This workspace supports debugging on both Windows and macOS. The main `launch.json` contains macOS configurations by default.

## Switching Platforms

To use Windows configurations:

1. Copy `.vscode/platforms/launch.windows.json` to `.vscode/launch.json`

To use macOS configurations:

1. Copy `.vscode/platforms/launch.macos.json` to `.vscode/launch.json`

Or simply replace the contents of `.vscode/launch.json` with the appropriate platform file.

## Platform-Specific Details

### Windows (cppvsdbg)

- Build directory: `build/msvc/`
- Executables: `.exe` extensions
- Debugger: Visual Studio C++ Debugger

### macOS (lldb)

- Build directory: `build/macos-qt6/`
- Executables: No extensions
- Debugger: LLDB (requires CodeLLDB extension)

## Available Configurations

Both platforms provide:

- **Debug EKiosk**: Debug the main client application
- **Debug Tray**: Debug the tray/watch service controller
- **Run EKiosk**: Run the client without debugging
- **Debug WatchService**: Debug the guard service
