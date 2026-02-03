# Platform Compatibility

EKiosk is designed to be cross-platform and works on:

- **Windows 7**: Qt 5.15 LTS (VC toolset 142) - transitional support only
- **Windows 10/11**: Qt 6.8 LTS (MSVC, Ninja)
- **Linux**: Qt 6.8 LTS (GCC, Clang, Ninja)
- **macOS**: Qt 6.8 LTS (Clang, Ninja) - app bundles with automatic code signing

## Qt Version Strategy

EKiosk uses platform-specific Qt versions for optimal compatibility:

- **Windows 7**: Qt 5.15 LTS with VC toolset 142 (transitional support)
- **Windows 10+ and Linux**: Qt 6.8 LTS
- **macOS**: Qt 6.8 LTS

## Notes

- Use CMake presets for platform-specific builds.
- Qt version is automatically detected based on the target platform.
- macOS builds create signed app bundles for proper execution.
- Windows 7 builds use Qt 5.15 LTS with VC toolset 142.
- Windows 10+ and Linux builds use Qt 6.8 LTS.
- Some device drivers may require platform-specific code.

## macOS Development Notes

For macOS development and debugging:

- **Code Signing**: Applications are automatically signed with ad-hoc signing for development. This allows debugging without proper Apple Developer certificates.
- **Qt Code Signing Validation**: To prevent Security framework validation errors during debugging, set `QT_MAC_DISABLE_CODE_SIGNING_CHECK=1` before creating QApplication:

  ```cpp
  qputenv("QT_MAC_DISABLE_CODE_SIGNING_CHECK", "1");
  QApplication::setAttribute(Qt::AA_DisableSessionManager);
  ```

- **App Bundles**: GUI applications are built as `.app` bundles. Run the executable at `Contents/MacOS/<appname>` inside the bundle.
- **Debugging**: Use LLDB or VS Code with CodeLLDB extension for debugging macOS applications.

### LLDB Debugging Configuration

Due to a known issue with CMake Tools on macOS (see [vscode-cmake-tools#3908](https://github.com/microsoft/vscode-cmake-tools/issues/3908)), the system LLDB does not support the Machine Interface (MI) mode required by VS Code's debugger. To enable debugging:

1. **Configure CMake Tools**: Add the following to `.vscode/settings.json`:

   ```json
   "cmake.debugConfig": {
     "type": "cppdbg",
     "MIMode": "lldb",
     "miDebuggerPath": "/Users/<username>/.vscode/extensions/ms-vscode.cpptools-<version>-darwin-arm64/debugAdapters/lldb-mi/bin/lldb-mi"
   }
   ```

   Replace `<username>` with your macOS username and `<version>` with your C++ extension version (e.g., `1.30.3`).

2. **Alternative**: If the above doesn't work, replace the system LLDB binary (not recommended):

   ```bash
   sudo mv /Library/Developer/CommandLineTools/usr/bin/lldb /Library/Developer/CommandLineTools/usr/bin/lldb.original
   sudo ln -s /Users/<username>/.vscode/extensions/ms-vscode.cpptools-<version>-darwin-arm64/debugAdapters/lldb-mi/bin/lldb-mi /Library/Developer/CommandLineTools/usr/bin/lldb
   ```

   To restore: `sudo mv /Library/Developer/CommandLineTools/usr/bin/lldb.original /Library/Developer/CommandLineTools/usr/bin/lldb`

## macOS Icon Guidelines

### Template Icons for Theme Support

For icons that need to adapt to light/dark theme switching:

1. **SVG Template Naming**: Name SVG source files with "Template" suffix (e.g., `iconTemplate.svg`)
2. **Qt Icon Creation**: Use `QIcon(":/icons/iconTemplate.png")` and call `icon.setIsMask(true)`
3. **Automatic Inversion**: macOS automatically inverts template icons based on theme
4. **Black Fill**: Use solid black (#000000) fill in SVG templates for best results
5. **Disabled State**: Let Qt handle disabled state automatically - do not create custom disabled pixmaps
6. **Color Elements**: Template icons should be monochrome - colored elements (like red dots) will not invert properly

### Template Icon Limitations

- **Monochrome Only**: Template icons are inverted as a whole - colored elements don't adapt meaningfully
- **Status Indicators**: For colored status indicators, consider using separate non-template icons or different visual approaches
- **Context Menus**: Disabled menu item icons may not follow template behavior - test thoroughly

### Bundle Configuration

- Set `LSUIElement = YES` in Info.plist for status bar only apps
- Include `CFBundleIconFile` pointing to .icns file for Finder display
- Use `macdeployqt` for proper framework bundling

### Icon Generation

- Use the project's `scripts/generate_icons.py` script to generate PNG/ICO/ICNS files from SVG templates
- All generated icon files are automatically included in Qt resources via Resources.qrc
- Template icons are identified by the "Template" suffix in filenames
