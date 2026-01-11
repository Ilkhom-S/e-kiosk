# EKiosk Tests

This folder will contain unit and integration tests for all EKiosk apps and shared modules.

- Mirror the structure of `src/` and `apps/` for test coverage.
- Use CMake to add test targets for each app/module.
- Use Qt Test or your preferred C++ testing framework.

## Adding Tests

1. Create a subfolder for each app or module (e.g., `kiosk/`, `Updater/`, `shared/`).
2. Add test source files (e.g., `test_main.cpp`, `test_<feature>.cpp`).
3. Add a `CMakeLists.txt` in each subfolder to define test targets.
4. Update the top-level `tests/CMakeLists.txt` to add all test subdirectories.

## Running Tests

- Configure and build with CMake.
- Use `ctest` or your IDE to run tests.

---

See the main docs for test strategy and coverage requirements.
