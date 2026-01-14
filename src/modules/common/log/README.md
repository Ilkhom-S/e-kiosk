# Logging (Common)

This folder contains the implementation of the project logging utilities.

- Canonical docs: `../../../docs/modules/logging.md`

## Structure (implementation)

```text
src/modules/common/log/
├── CMakeLists.txt          # Build configuration
├── src/
│   ├── SimpleLog.cpp       # Implementation of ILog and file rotation
│   ├── LogManager.cpp      # Logger lifecycle and registry
│   └── ...                 # helpers
└── include/
    └── Common/
        └── ILog.h          # Public interface
```

**Contributor notes:**

- Keep usage examples and user-facing docs in `docs/modules/logging.md`.
- Use this README to document internal APIs, file layout and testing notes.
