# Modularization Migration TODO

This document tracks the migration from a monolithic to a modular architecture.

## Tasks

- [x] Move all code from src/ to apps/kiosk/
- [ ] Refactor shared code into src/ and include/
- [ ] Update CMake to build each app and link shared code
- [ ] Mirror src/ structure in tests/
- [ ] Update documentation for each migration step
- [ ] Review and clean up thirdparty/ dependencies
- [ ] Add CI for modular builds and tests

## Notes

- All breaking or structural changes must be reflected in the docs.
- Keep this list updated as migration progresses.
