# DebugUtils

This folder contains the implementation for debugging utilities used by EKiosk.

- Canonical docs: `../../docs/modules/debugutils.md`

## Structure (implementation)

```text
src/modules/DebugUtils/
├── CMakeLists.txt          # Build configuration
└── src/
    ├── DebugUtils.cpp      # Main implementation
    ├── QStackWalker.h      # Stack walking wrappers
    ├── StackWalker.cpp     # Third-party stack walker (modified)
    └── ...                 # tests and helpers
```

**Contributor notes:**

- Keep high-level docs and examples in `docs/modules/debugutils.md`.
- Use this README for implementation notes, platform dependencies and testing guidance.
