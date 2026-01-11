# Platform Compatibility

EKiosk is designed to be cross-platform and works on:

- Windows 10/11 (MSVC, Ninja)
- Linux (GCC, Clang, Ninja)
- (Planned) macOS support

## Notes

- Use CMake presets for platform-specific builds.
- Ensure Qt version matches your compiler (e.g., MSVC Qt for MSVC builds).
- Some device drivers may require platform-specific code.
