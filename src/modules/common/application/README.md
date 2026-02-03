# BaseApplication (Common)

Implementation notes for the BaseApplication module.

- Canonical docs: `../../../docs/modules/base_application.md`

## Structure (implementation)

```text
src/modules/common/application/
├── CMakeLists.txt              # Build configuration
├── src/
│   └── BasicApplication.cpp    # Implementation (public interface in include/Common/BasicApplication.h)
└── include/
    └── Common/
        ├── BasicApplication.h  # Public interface
        └── BasicApplication.tpp # Template implementations
```

**Contributor notes:**

- Keep user-facing docs and examples in `docs/modules/base_application.md`.
- Use this README for implementation notes, API stability promises, and test hints.
