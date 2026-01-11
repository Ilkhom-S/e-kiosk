# Coding Standards

## General

- Use C++14 or later.
- Follow Qt coding conventions for signals/slots, naming, and file structure.
- Each class should have a .cpp/.h pair.
- UI files should use .ui and be edited with Qt Designer.

## Formatting

- Indent with 4 spaces.
- Use braces for all control blocks.
- Use descriptive variable and function names.

## Commits

- Use [Conventional Commits](https://www.conventionalcommits.org/).
- Document all breaking or significant changes in the docs.

## CMake Project Structure

- **Always use the EKiosk CMake helpers** (`ek_add_application`, `ek_add_library`, `ek_add_plugin`, `ek_add_test`, etc.) for all new and refactored targets. This ensures consistency, maintainability, and DRY principles across the project.
- See [cmake/README.md](../cmake/README.md) for usage and examples.

## Documentation

- Update or create relevant docs for any new features, refactors, or breaking changes.
- Link related docs for easy navigation.
