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
