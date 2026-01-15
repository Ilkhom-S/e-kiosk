# UpdateEngine

This folder contains the module implementation. The full, canonical documentation has been moved to the central docs site:

- See: `../../../docs/modules/updateengine.md`

## Structure (implementation)

```text
src/modules/UpdateEngine/
├── CMakeLists.txt                  # Module build configuration
├── include/UpdateEngine/           # Public headers (installed targets)
│   ├── UpdateEngine.h
│   └── ...
├── src/
│   ├── UpdateEngine.cpp            # Main update management
│   ├── UpdateManifest.cpp          # XML manifest parser
│   ├── UpdateTask.cpp              # Individual update tasks
│   └── ...                         # internal helpers, tests, etc.
└── README.md                       # This file (short pointer to canonical docs)
```

**Contributor notes:**

- Keep high-level documentation, examples, and usage in `docs/modules/updateengine.md`.
- Use this README for implementation notes, file layout, build or testing hints and internal design details.
- For API docs, examples, and user-facing information, see the canonical docs.

## Qt version handling

This module handles Qt version differences for XML parsing:

- **Qt5**: Uses `XmlPatterns` module for XQuery/XPath operations
- **Qt6**: `XmlPatterns` is deprecated; uses alternative XML parsing

When modifying XML-related code, ensure compatibility with both Qt versions.

## API summary (short)

- **Primary classes:** `UpdateEngine`, `UpdateManifest`, `UpdateTask`, `UpdateInstaller`.
- **Key signals:** `updateAvailable()`, `updateProgress(int, QString)`, `updateCompleted()`.
- **Qt compatibility:** Handles XmlPatterns deprecation in Qt6.

Keep this summary updated when APIs change so reviewers can quickly spot relevant changes.
