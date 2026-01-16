# DebugUtils

This folder contains the implementation for debugging utilities used by EKiosk.

- Canonical docs: `../../docs/modules/debugutils.md`

## Structure (implementation)

```text
src/modules/DebugUtils/
├── CMakeLists.txt          # Build configuration
└── src/
    ├── DebugUtils.cpp      # Main implementation (Boost.Stacktrace)
    ├── QStackWalker.h      # Legacy stack walking wrappers (deprecated)
    ├── StackWalker.cpp     # Legacy third-party stack walker (deprecated)
    └── ...                 # tests and helpers
```

## Recent Changes (2026)

**Migration to Boost.Stacktrace:** The module has been modernized to use Boost.Stacktrace instead of the legacy Windows DbgHelp API. This provides:

- Cross-platform support (Windows, Linux, macOS)
- Better performance and reliability
- Modern C++ API
- Automatic crash log generation

**Legacy Code:** The old `StackWalker.*` and `QStackWalker.h` files are kept for compatibility but are no longer used. They may be removed in a future version.

**Contributor notes:**

- Keep high-level docs and examples in `docs/modules/debugutils.md`.
- Use this README for implementation notes, platform dependencies and testing guidance.
- The module now depends on Boost.Stacktrace instead of Windows-specific APIs.
