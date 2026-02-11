# Deprecated Code

This folder contains code that has been deprecated and is no longer used in the main EKiosk application.

## Contents

### ekiosk-src/

Contains deprecated code originally from `apps/EKiosk/src/`:

- **db/** - Database module (deprecated, use DatabaseProxy module instead)
- **devices/** - Device module (deprecated, use Hardware/Common module instead)
- **modules/** - Old modules from TerminalClient (deprecated, being refactored into modular system)
- **other/** - Miscellaneous utilities (deprecated)
- **ui/** - Old UI code (deprecated, use GraphicsEngine instead)
- **updater/** - Update module (deprecated, use UpdateEngine module instead)
- **connection/** - Connection module (deprecated, use Connection module instead)
- **mainwindow.h/cpp/ui** - Old main window (deprecated)

## Build Status

These folders are **NOT** compiled by default. To include them in the build, use:

```bash
cmake -S . -B build -DBUILD_DEPRECATED=ON
```

## Migration Notes

- The modular refactoring of EKiosk is ongoing
- New code should use the modules in `src/modules/` and `include/` instead of these deprecated folders
- These folders are kept for reference and git history preservation only
- Target for complete removal: After full modularization is complete

## References

- See [docs/migration-todo.md](../docs/migration-todo.md) for the migration plan
- See [docs/migration-plan.md](../docs/migration-plan.md) for architecture refactoring details
