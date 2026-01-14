# Testing Guide 🧪

This guide explains where tests live, how to run them locally, and how to add new tests to the EKiosk project.

## Overview

- Tests live under `tests/` and are organized by scope:
  - `tests/unit/` — small, fast unit tests
  - `tests/modules/<Module>/` — module-specific tests (integration/unit for that module)
  - `tests/` can contain other integration/system tests
- Tests use the project's CMake helpers. Prefer `ek_add_test()` for adding test targets so CI and the top-level test target pick them up automatically.

## Running tests locally

1. Build the project (configure first if needed):

```bash
cmake --preset <your-preset>
cmake --build build/<your-preset>
```

1. Run all tests via CTest:

```bash
# Run all tests
cmake --build build/<your-preset> --target test
# Or using ctest for filtering and verbosity
cd build/<your-preset>
ctest -V
```

1. Run a subset by name or regex:

```bash
# Run module tests matching 'NetworkTaskManager'
ctest -R NetworkTaskManager -V
# Run a single test by exact name
ctest -R TestQFileLogger -V
```

Notes:

- Use `-V` for verbose output and to see test stdout/stderr.
- On CI prefer `ctest -j <N>` to run tests in parallel when supported.

## Running a single test executable

If you need to run or debug a single test binary, find it in `build/<preset>/tests/...` and execute it directly, or use `ctest -R` to run by name.

## Adding tests

- Place test sources under `tests/unit/` or `tests/modules/<Module>/`.
- Add the test target with `ek_add_test()` in the appropriate CMakeLists.txt so it integrates with top-level test target and CI.

Example CMake fragment:

```cmake
ek_add_test(TestNetworkTaskManager
    SOURCES tests/modules/NetworkTaskManager/TestThread.cpp
    LIBRARIES NetworkTaskManager
)
```

## Test naming & style

- Test file names should start with `Test` (e.g., `TestQFileLogger.cpp`, `TestThread.cpp`).
- Prefer small, deterministic unit tests. Avoid external network or filesystem dependencies unless the test is explicitly an integration test and documented as such.
- Mark long-running or flaky tests in their docs so CI can treat them specially.

## Integration & environment notes

- If tests require environment variables, services, or network endpoints, document them in the module's `docs/modules/<module>.md` under the **Testing** section.
- For tests that require native libraries (e.g., OpenSSL), document any platform package prerequisites.

## CI and PRs

- CI runs the test suite on each PR; all tests must pass before merging.
- If a change adds or modifies tests, include a brief note in the PR description about the new or changed tests.

## Coverage (local & CI)

We provide a coverage build option and a CI job that produces an HTML coverage report as an artifact.

### Local coverage

1. Configure with coverage enabled:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
cmake --build build -j
```

1. Run the tests (ctest or via the `test` target):

```bash
cmake --build build --target test
# or
cd build && ctest -V
```

1. Generate coverage:

```bash
lcov --directory build --capture --output-file build/coverage.info
lcov --remove build/coverage.info '/usr/*' --output-file build/coverage.info
genhtml build/coverage.info --output-directory build/coverage-report
# open build/coverage-report/index.html in a browser

```

> Requirements: `lcov` and `genhtml` must be installed (e.g., `sudo apt install lcov genhtml`).

### CI coverage

- The CI job `coverage` runs on Ubuntu and produces an HTML coverage report uploaded as an artifact (`coverage-report`).
- Optionally, if `CODECOV_TOKEN` is configured in the repository secrets, CI will upload `coverage.info` to Codecov.

## Troubleshooting

- Use `ctest -V -R <pattern>` to see verbose logs for failing tests.
- Use gdb/visual studio debugger against the test binary for crashes.

---

I added a small test template under `tests/modules/example/TestTemplate.cpp` and an accompanying `CMakeLists.txt` (`tests/modules/example/CMakeLists.txt`). Add more targeted templates into the appropriate `tests/modules/<Module>/` directories as needed.
