# Modularization Migration TODO

This document tracks the migration from a monolithic to a modular architecture.

## Tasks

- [x] Move all code from src/ to apps/kiosk/
- [ ] Refactor shared code into src/ and include/
- [x] Add `BaseApplication` module under `src/modules/common` and port `apps/kiosk` to use it
- [ ] Update CMake to build each app and link shared code
      Notes:
- `SingleApplication` is now vendored as a git submodule under `thirdparty/SingleApplication` to avoid pulling Qt6 via vcpkg. Use `git submodule update --init --recursive` after cloning.- [ ] Mirror src/ structure in tests/
- [ ] Update documentation for each migration step
- [ ] Review and clean up thirdparty/ dependencies
- [ ] Add CI for modular builds and tests

## Notes

- All breaking or structural changes must be reflected in the docs.
- Keep this list updated as migration progresses.
