# Getting Started with EKiosk Development

Welcome to the EKiosk project! This guide will help you set up your development environment and get started quickly.

## Prerequisites

- Qt 5.15.x (or later) with Qt Widgets, SerialPort, WebSockets, etc.
- CMake 3.16+
- Ninja or MSVC (Windows) or GCC/Clang (Linux)
- Git
- Recommended: Visual Studio Code or Qt Creator

## Quick Start

1. **Clone the repository:**

   ```
   git clone <repo-url>
   cd ekiosk
   ```

2. **Configure Qt Kits:**
   - Use the VS Code Qt Extension or Qt Creator to register your Qt installation.
   - Ensure the correct kit is selected for your platform (MSVC, MinGW, GCC, etc).
3. **Configure the project:**

   ```
   cmake --preset <your-preset>
   ```

   - See `CMakePresets.json` for available presets.

4. **Build:**

   ```
   cmake --build build/<your-preset>
   ```

5. **Run:**
   - Launch the desired app from `apps/` (e.g., `apps/kiosk/`).
   - Use your IDE's debugger or run the executable directly.

## Documentation

- See [architecture.md](architecture.md) for project structure.
- See [build-guide.md](build-guide.md) for detailed build instructions.
- See [coding-standards.md](coding-standards.md) for code style and conventions.
- See [migration-plan.md](migration-plan.md) for ongoing modularization efforts.

## Contributing

- Follow the [conventional commits](https://www.conventionalcommits.org/) standard.
- Update documentation for any breaking or significant changes.
- See [copilot-instructions.md](../.github/copilot-instructions.md) for AI agent guidelines.

## Support

For help, open an issue or contact the maintainers.
