# Platform Compatibility

EKiosk is designed to be cross-platform and works on:

- **Windows 7**: Qt 5.15 LTS (VC toolset 142) - transitional support only
- **Windows 10/11**: Qt 6.8 LTS (MSVC, Ninja)
- **Linux**: Qt 6.8 LTS (GCC, Clang, Ninja)
- (Planned) macOS support

## Qt Version Strategy

EKiosk uses platform-specific Qt versions for optimal compatibility:

- **Windows 7**: Qt 5.15 LTS with VC toolset 142 (transitional support)
- **Windows 10+ and Linux**: Qt 6.8 LTS

## Notes

- Use CMake presets for platform-specific builds.
- Qt version is automatically detected based on the target platform.
- Windows 7 builds use Qt 5.15 LTS with VC toolset 142.
- Windows 10+ and Linux builds use Qt 6.8 LTS.
- Some device drivers may require platform-specific code.
