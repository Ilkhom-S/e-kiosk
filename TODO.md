# TODO

## Migration Tasks

- [x] Create qt4-fallback branch
- [x] Update .pro for Qt 5
- [x] Migrate build system to CMake
- [x] Integrate clang-tidy for static analysis
- [x] Set up clang-format for code styling
- [ ] Review code for Qt 4 deprecated APIs
- [ ] Test build with Qt 5
- [ ] Update documentation

## Development Tasks

- [ ] Add unit tests for modules
- [ ] Implement logging improvements
- [ ] Refactor device classes for better abstraction
- [ ] Add error handling for network failures
- [ ] Optimize UI responsiveness

- [ ] Refactor explorer.exe (Windows shell) killing logic:
  - Remove automatic killing of explorer.exe on startup.
  - Make shell restriction (killing/hiding explorer.exe) runtime-configurable and only available to developers.
  - Add UI control (e.g., toggle or button) to enable/disable shell restriction, similar to TCPKiosk's /aff mode.
  - Ensure this feature is never enabled by default in production.
  - Document safe usage and developer-only access.

## Documentation

- [x] Create README.md
- [x] Create docs/architecture.md
- [x] Create docs/migration-plan.md
- [ ] Add API documentation
- [ ] Create user manual

## Maintenance

- [ ] Update dependencies
- [ ] Fix any linting issues
- [ ] Review security practices
