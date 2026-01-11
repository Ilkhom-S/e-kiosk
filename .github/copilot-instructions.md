## Conventional Commits & Scopes

All commits must follow the [Conventional Commits](https://www.conventionalcommits.org/) standard.

**Recommended scopes for this project:**

- apps/kiosk, apps/updater, apps/guard
- src, include, thirdparty, tests, docs, cmake
- devices, modules, ui, connection, db, other, updater
- build, ci, config, migration

**Examples:**

- feat(apps/kiosk): add new payment module
- fix(devices): resolve printer timeout issue
- docs(getting-started): update setup instructions

Use clear, descriptive scopes to indicate which part of the project is affected.

# EKiosk Qt C++ Project - AI Coding Agent Instructions

## Folder Structure (2026 Modular Redesign)

- **apps/**: Contains all executable applications (e.g., kiosk, updater, guard). Each app has its own folder.
  - Each app (e.g., `kiosk`) has its own `src/` for code, and a `CMakeLists.txt` at the app root.
  - Platform/build folders (e.g., `msvc/`) can also be placed at the app root.
- **src/**: Shared code and libraries, to be refactored from apps/kiosk/src as modularization proceeds.
- **include/**: Public headers for use across modules and apps.
- **thirdparty/**: External/third-party libraries.
- **tests/**: Unit and integration tests for all apps and modules.
  - Mirrors the structure of `src/` and `apps/` for test coverage.
  - Each app/module can have its own test subfolder (e.g., `tests/kiosk/`, `tests/Updater/`).
  - Use CMake to add test targets for each app/module; top-level `tests/CMakeLists.txt` adds all test subdirectories.
  - Use Qt Test or another C++ testing framework.
- **resources/**: Images, sounds, styles, etc.
- **docs/**: Project documentation.
- **cmake/**: Custom CMake modules.

## Documentation Requirements

- Any change that breaks, restructures, or introduces new features **must** be reflected in the relevant docs (architecture, build-guide, coding-standards, migration-todo, etc).
- Keep documentation up to date and link between related docs for easy navigation.

## Migration Plan

- Move all code from `src/` to `apps/kiosk/` as a first step.
- Refactor shared code into `src/` and `include/` as modularization proceeds.
- Update CMake to build each app and link shared code.
- Track migration progress in [docs/migration-todo.md](../docs/migration-todo.md).

## Best Practices

- Keep apps, shared code, and third-party libraries clearly separated.
- Use CMake targets for each app and library.
- Mirror src/ structure in tests/ for coverage.
- Follow conventional commits and update docs for all major changes.

## References

- [Getting Started](../docs/getting-started.md)
- [Build Guide](../docs/build-guide.md)
- [Coding Standards](../docs/coding-standards.md)
- [Platform Compatibility](../docs/platform-compatibility.md)
- [Multilanguage Support](../docs/multilanguage.md)
- [Migration Plan & TODO](../docs/migration-todo.md)

---

_For any major change, update the docs and migration-todo list!_
