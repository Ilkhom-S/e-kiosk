# EKiosk Qt5/Qt6 Scripting Migration Plan

This migration plan outlines the steps required to make EKiosk scripting features compatible with both Qt5 (QtScript) and Qt6 (QJSEngine). Follow these steps to ensure a smooth transition and maintain cross-version support.

---

## Step 1: Audit and Identify QtScript Usage

- Search for all usages of `QScriptEngine`, `QScriptValue`, `QtScript`, and related CMake `find_package(Qt5Script)` calls.
- Document affected files (e.g., `JSScenario.h/.cpp`, `ScriptArray.h`, `AdService.h`, CMakeLists.txt, plugin docs).

## Step 2: Abstract Scripting Engine

- Create a common interface/class (e.g., `IScriptingEngine`) that wraps script evaluation, value conversion, and object exposure.
- Implement two backends:
  - Qt5: Uses `QScriptEngine`
  - Qt6: Uses `QJSEngine`
- Use preprocessor guards (`#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)`) to select implementation.

## Step 3: Refactor CMake Configuration

- Update all CMakeLists.txt files:
  - Use `find_package(Qt${QT_VERSION_MAJOR}Script)` for Qt5 only.
  - For Qt6, ensure `QtQml` is found and linked.
  - Guard code and plugin builds that require scripting.

## Step 4: Update Headers and Source Files

- Replace direct `QScriptEngine`/`QScriptValue` usage with the new abstraction.
- Update includes:
  - Qt5: `<QtScript/QScriptEngine>`
  - Qt6: `<QtQml/QJSEngine>`
- Use module-qualified Qt includes as per coding standards.

## Step 5: Port Scripting Logic

- For each affected class (e.g., `JSScenario`, `ScriptArray`, `AdService`):
  - Port script evaluation, value conversion, and object registration to work with both engines.
  - Document API differences and add Russian comments as required.

## Step 6: Update Plugin and Module Documentation

- Update `README.md` and `docs/plugins/` to reflect scripting engine changes.
- Add migration notes and usage examples for both Qt5 and Qt6.

## Step 7: Comprehensive Testing

- Build and run tests for both Qt5 and Qt6 configurations.
- Ensure all scripting features work as expected.
- Update or add tests for new abstraction.

## Step 8: Finalize and Document Migration

- Update `docs/architecture.md` and `docs/migration-todo.md` with migration details.
- List any deprecated APIs and new compatibility notes.
- Ensure all contributors are aware of the new scripting abstraction and usage.

---

**Notes:**
Finalize and Document Migration

- Update `docs/architecture.md` and `docs/migration-todo.md` with migration details.
- List any deprecated APIs and new compatibility notes.
- Ensure all contributors are aware of the new scripting abstraction and usage.

---

**Notes:**

- This plan ensures a clean, maintainable migration with minimal disruption.
- Tackle each step incrementally, verifying builds and tests at each stage.
- Russian comments and file header comments should be added per coding standards.

---
