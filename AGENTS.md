# Coding Standards

## AI Agent Response Guidelines

**CONCISE MODE:** When reporting completed tasks or jobs, provide short answers/results only. Use minimal text with ✅ checkmarks for confirmations and avoid verbose explanations unless specifically requested. Focus on what was accomplished, not how.

Example:

- ✅ **Updated file successfully** - Brief confirmation
- ❌ **Error occurred** - Brief issue description
- 📋 **Summary**: 3 files changed, 15 lines modified - Key metrics only

## INI Templates and Russian Documentation

- All automatically generated or template-based .ini files for application configuration (e.g., controller.ini, ekiosk.ini) **must include documentation and comments in Russian**. This is required because most users and operators do not read English.
- When generating or updating ini templates (e.g., via CMake or in runtimes/common/data/), ensure all parameter descriptions, section headers, and usage notes are in Russian.
- When documenting configuration keys in README.md or other user-facing docs, provide Russian-language examples and explanations for ini files.
- If you add new required keys for an app, update the corresponding .ini template and its Russian comments.
- See runtimes/common/data/controller.ini.in for an example of a fully documented Russian-language ini template.

## C++ Guidelines

- **Use Qt Types:** Prefer QString, QList, QMap over STL equivalents.
- **Smart Pointers:** Use QScopedPointer, QSharedPointer, or std::unique_ptr as appropriate.
- **Naming Conventions:**
  - Classes: PascalCase (e.g., DeviceManager)
  - Methods: camelCase (e.g., initializeDevice)
  - Member variables: mCamelCase (e.g., mDeviceList)
  - Constants: UPPER_SNAKE_CASE or kPascalCase
  - Interfaces: Prefix with I (e.g., IDevice, ICryptEngine)
- **Header Guards:** Use #pragma once
- **Comments for Adopted/Platform-Specific Code:** When adopting code from other projects or implementing platform-specific behavior (e.g., #ifdef Q_OS_WIN), add clear comments explaining the rationale, origin, and why it's necessary. Use prefixes like "Note:", "Adopted from [repo]:", or "Platform-specific:" to aid future maintainers. This ensures non-obvious decisions are documented for cross-platform compatibility and code evolution.
- **Russian Comments for Code Sections:** When a method or block contains multiple independent actions (e.g., initialization steps, logging sequences), add Russian comments above the block to describe the overall purpose, e.g., "// Выводим стандартный заголовок в лог" for logging headers. This follows the original TerminalClient style for clarity in multi-step operations.
- **Function Comments:** Place Russian comments above each function declaration in the style `//---------------------------------------------------------------------------` followed by `// [Description in Russian]`, matching the TerminalClient convention for consistency. For example:

  ```cpp
  //---------------------------------------------------------------------------
  // Возвращает настройки приложения
  QSettings &BasicApplication::getSettings() const
  {
      return *m_settings;
  }
  ```

- **File Header Comments:** All header (.h/.hpp) and implementation (.cpp/.cc) files must start with a Russian comment in the format `/* @file [Description in Russian]. */` to describe the file's purpose. For example:

  ```cpp
  /* @file Шаблон объявления плагина. */
  ```

  or

  ```cpp
  /* @file Реализация клиента, взаимодействующего с сервером рекламы. */
  ```

- **Method Comments in Headers:** All public and protected methods in header files should be commented in Russian where possible, using the same style as function comments (//--------------------------------------------------------------------------- followed by // [Description in Russian]).

- **Constants and Variables Comments:** Constants, static variables, and other declarations should be commented with `///` for descriptions, followed by inline comments if needed. For example:

  ```cpp
  namespace CConnection {
      /// Период проверки статуса соединения.
      const int DefaultCheckPeriod = 60 * 1000; // 1 минута

      /// Период пинга соединения.
      const int DefaultPingPeriod = 15 * 60 * 1000; // 15 минут

      /// Таймаут запроса проверки соединения.
      const int PingTimeout = 30 * 1000; // 30 секунд

      /// Хост по умолчанию для проверки соединения.
      const QString DefaultCheckHost = "http://mon.humo.tj:80/ping";

      /// Строка ответа по умолчанию для проверки соединения.
      const QString DefaultCheckResponse = "";
  } // namespace CConnection
  ```

- **Complex Logic Comments in CPP Files:** In implementation files, if logic is complex or a developer may not understand, place Russian comments explaining what's happening, e.g., "// Планируем перезагрузку ЕК по истечении рекламы."
- **Conditional Compilation Comments:** When using `#if QT_VERSION`, `#ifdef`, or other preprocessor conditionals (especially for Qt version differences), always add Russian comments above explaining why the conditional is needed and what each branch does. Reference sources if applicable. Example:

  ```cpp
  // Объединяем контексты: в Qt6 метод unite() был удален, используем insert()
  // Qt5: unite() - объединяет карты, перезаписывая существующие ключи
  // Qt6: insert() - аналогичная функциональность после удаления unite()
  #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
      mContext.insert(aContext);
  #else
      mContext.unite(aContext);
  #endif
  ```

## Header Inclusion Style

- **Angle Brackets (`<>`):** Use for all headers located in the project's global `include/` directories or module interfaces (e.g., `#include <Hardware/Common/VirtualDeviceBase.h>`). This treats internal modules as library components.
- **Double Quotes (`""`):** Use **only** for "private" headers located in the same immediate directory as the source file (e.g., `#include "LocalHelper_p.h"`).
- **Refactoring:** When modifying or refactoring files, automatically convert relative path includes (e.g., `#include "../../Common/file.h"`) to the bracketed absolute-path style relative to the include root.- **Refactoring:** When modifying or refactoring files, automatically convert relative path includes (e.g., `#include "../../Common/file.h"`) to the bracketed absolute-path style relative to the include root.

- Never ignore `-Wtautological-compare` warnings.
- Example: `a != a` is always false; investigate the logic to find the correct comparison target.

### Template File Placement

- **Location:** Place `.tpp` files in the same directory as their corresponding `.h` file within the `include/` tree.
- **Inclusion:** At the very end of the `.h` file, after the class closing brace and any namespaces, add `#include <Path/To/File.tpp>`.
- **CMake:** Do not list `.tpp` files as compilation units in `add_library`, but include them in `target_sources` for IDE visibility.- **CMake:** Do not list `.tpp` files as compilation units in `add_library`, but include them in `target_sources` for IDE visibility.

## Cross-Platform and Qt Version Compatibility Requirements

**CRITICAL REQUIREMENT:** All code changes must be compatible with Linux, macOS, and Windows, and support both Qt 5.15+ and Qt 6.x.

### Platform Compatibility

- **Target Platforms:** Linux, macOS, Windows
- **Platform Detection:** Use standard CMake platform checks:
  - `if(WIN32)` for Windows-specific code
  - `if(APPLE)` for macOS-specific code
  - `if(UNIX AND NOT APPLE)` for Linux-specific code
- **Platform-Specific Code:** When implementing platform-specific behavior:
  - Document the rationale clearly with comments
  - Use appropriate preprocessor guards
  - Provide fallback behavior for unsupported platforms when possible
  - Example:

    ```cpp
    // Windows-specific USB handling
    #ifdef Q_OS_WIN32
        // Windows USB implementation
        mUSBPort = new WindowsUSBPort();
    #else
        // Cross-platform fallback or stub
        mUSBPort = nullptr; // USB not available on this platform
    #endif
    ```

### Qt Version Compatibility

- **Supported Qt Versions:** Qt 5.15+ and Qt 6.x
- **Qt Version Detection:** Use `QT_VERSION` macros for version-specific code:

  ```cpp
  #if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
      // Code for Qt 5.14+ and Qt 6.x (QRecursiveMutex available)
      mMutex = QRecursiveMutex();
  #else
      // Fallback for Qt 5.0-5.13
      mMutex = QMutex(QMutex::Recursive);
  #endif
  ```

- **Qt API Changes:** When APIs change between Qt versions:
  - Prefer the newer API when available
  - Provide backward-compatible fallbacks
  - Document the version requirements clearly
  - Examples of handled changes:
    - `QTextCodec` → Custom `CodecBase` class (Qt6 removed QTextCodec)
    - `QRegExp` → `QRegularExpression` (Qt6 removed QRegExp)
    - `QTime::restart()` → `QElapsedTimer` (Qt6 removed QTime methods)
    - `QMutex::Recursive` → `QRecursiveMutex` (Qt6 removed QMutex::Recursive)

### Cross-Platform Qt Guidelines

- **Qt Platform Plugins:** Ensure appropriate platform plugins are available
- **File Paths:** Use QDir::separator() or "/" for cross-platform paths
- **Line Endings:** Qt handles line ending conversion automatically
- **Unicode Support:** All strings use UTF-8 encoding consistently
- **Threading:** Use Qt's threading classes (QThread, QMutex, etc.) for portability

### Testing Requirements

- **Platform Testing:** Code must be tested on all supported platforms when possible
- **Qt Version Testing:** Test builds with both Qt 5.15+ and Qt 6.x
- **Conditional Logic Testing:** Test both branches of conditional compilation
- **Fallback Behavior:** Verify fallback implementations work correctly

### Documentation Requirements

- **Platform Notes:** Document platform-specific behavior and limitations
- **Qt Version Notes:** Document minimum Qt version requirements and API differences
- **Conditional Logic:** Explain all `#ifdef` and `#if QT_VERSION` blocks with comments
- **Migration Notes:** Document any breaking changes or migration requirements

### Implementation Checklist

Before committing changes:

- [ ] Code builds successfully on Linux, macOS, and Windows
- [ ] Code compiles with Qt 5.15+ and Qt 6.x
- [ ] Platform-specific code is properly guarded and documented
- [ ] Qt version-specific code uses appropriate version checks
- [ ] Fallback implementations provided for unsupported platforms/features
- [ ] All conditional compilation is clearly commented
- [ ] Tests pass on all supported platforms and Qt versions
- [ ] Documentation updated to reflect platform/Qt version requirements

- **Interface Separators:** If a class implements more than one interface or has multiple sections, separate them with `//--------------------------------------------------------------------------------` or use `#pragma region` and `#pragma endregion` for better organization.

- **Include Order:**
  - **Include Order:**
  1. Corresponding header (the header for this implementation file)
     #include "MyClass.h"
  2. Project headers and Qt headers
  - Wrap Qt headers between `Common/QtHeadersBegin.h` and `Common/QtHeadersEnd.h` to suppress Qt warnings on MSVC.
    #include "Common/QtHeadersBegin.h"
    #include <QtCore/QString>
    #include "Common/QtHeadersEnd.h"
  - Other project headers from `include/` come alongside these.
  1. Third-party headers
     #include <boost/optional.hpp>
  2. Standard library headers
     #include <vector>

  Example combined layout:

  // 1. Corresponding header
  #include "MyClass.h"

  // 2. Project headers (Qt headers wrapped)
  #include "Common/QtHeadersBegin.h"
  #include <QtCore/QString>
  #include "Common/QtHeadersEnd.h"

  // 3. Third-party headers
  #include <boost/optional.hpp>

  // 4. Standard library
  #include <vector>

## Qt Include Style

Prefer module-qualified Qt headers (e.g., `<QtCore/QString>`, `<QtWidgets/QWidget>`) rather than the unqualified `<QString>` form when possible.

Reasons:

- **Clarity:** makes the dependency explicit (which Qt module provides the symbol).
- **Forward-compatibility:** module-qualified paths align with Qt6 naming and reduce ambiguity when multiple modules provide similar symbols.
- **IDE/tooling:** helps language servers and indexers resolve headers more reliably.

Compatibility note:

- In Qt5 both forms are usually available; prefer `<QtCore/...>` style for consistency across Qt5/Qt6.

Guidance:

- Use `<QtCore/QString>` and similar in new code and when porting. If you encounter legacy code with `<QString>`, you may leave it unchanged unless you're refactoring includes broadly.
- Keep Qt headers wrapped with `Common/QtHeadersBegin.h` and `Common/QtHeadersEnd.h` on MSVC as described above.

### QtCore types

For types that belong to the QtCore module (examples: `QString`, `QByteArray`, `QVariant`, `QDateTime`, `QList`, `QMap`, `QSet`), include them explicitly from the QtCore module:

# include <QtCore/QString>

# include <QtCore/QByteArray>

This makes module ownership explicit and eases future Qt6 porting.

## Header Organization and Public-Private Separation

### When to Move Headers to `include/`

**Move header to `include/ModuleName/` if:**

- ✅ Used by multiple modules/plugins (shared API)
- ✅ Used by applications (`apps/`)
- ✅ Part of the public SDK interface
- ✅ Already has duplicate definitions in both `include/` and `src/`

**Keep header in `src/` if:**

- ✅ Only used within a single plugin (plugin-internal)
- ✅ Only used within a single module (module-internal)
- ✅ Not part of any public API
- ✅ Contains implementation details

### Standard C++ Project Structure (For Public Headers)

EKiosk follows the industry-standard C++ project structure for clear public API boundaries:

**Folder Layout (for headers moved to public):**

```
include/ModuleName/
  Header.h           ← PUBLIC HEADER (full interface definition)

src/modules/ModuleName/
  src/
    Header.h         ← DEPRECATED redirect (for compatibility only)
    Header.cpp       ← IMPLEMENTATION (methods only, no class definition)
```

**Key Rules:**

1. **Public headers** contain the full class/interface definition and live in `include/`
2. **Private implementation** contains only method bodies and lives in `src/modules/`
3. **For public headers, all includes** use the public path: `#include <ModuleName/Header.h>`
4. **Never use relative paths** like `#include "../../modules/..."` in any file

### Example: Scenario Module

**Public Interface** - `include/ScenarioEngine/Scenario.h`

```cpp
/* @file Базовый класс для сценариев. */
#pragma once

namespace GUI {
    class Scenario : public QObject, protected ILogable {
        Q_OBJECT
      public:
        Scenario(const QString &aName, ILog *aLog = 0);
        virtual ~Scenario();
        virtual void start(const QVariantMap &aContext) = 0;
        // ... full interface definition ...
    };
}
```

**Implementation** - `src/modules/ScenarioEngine/src/Scenario.cpp`

```cpp
/* @file Базовый класс для сценариев. */

// ALWAYS include the PUBLIC header
#include <ScenarioEngine/Scenario.h>

namespace GUI {
    Scenario::Scenario(const QString &aName, ILog *aLog)
        : mName(aName), mDefaultTimeout(0) {
        // implementation ...
    }
    // ... method implementations only ...
}
```

**Compatibility Redirect** - `src/modules/ScenarioEngine/src/Scenario.h`

```cpp
/* @file DEPRECATED - See include/ScenarioEngine/Scenario.h instead.

MIGRATION NOTE: This file kept for backward compatibility only.
The class definition has been moved to the public header in include/.
All NEW code should include <ScenarioEngine/Scenario.h>.
*/

#pragma once
#include <ScenarioEngine/Scenario.h>
```

### Header Migration Checklist

**Only follow this checklist when a header meets the "Move" criteria above.**

When refactoring a module from private headers to public headers:

1. **✅ Verify the header should be public**
   - Is it used across multiple modules/plugins?
   - Is it part of the public API?
   - Is there already a duplicate in `include/`?
   - If not, **leave it in `src/` and stop here**

2. **✅ Create/update public header** in `include/ModuleName/Header.h`
   - Full class definition
   - All public methods and signals/slots
   - Proper file header comment: `/* @file [Russian description]. */`
   - Use `#pragma once` for header guard

3. **✅ Implement in source file** - `src/modules/ModuleName/src/Header.cpp`
   - `#include <ModuleName/Header.h>` (use PUBLIC path, not local)
   - Only method implementations, no class definition
   - Same file header comment

4. **✅ Convert private header** - `src/modules/ModuleName/src/Header.h`
   - **KEEP IT** for backward compatibility
   - Replace entire content with:

     ```cpp
     /* @file DEPRECATED - See include/ModuleName/Header.h instead. */
     #pragma once
     #include <ModuleName/Header.h>
     ```

   - Add migration note explaining the change

5. **✅ Fix relative paths to public headers**
   - Only if the header already existed in `include/` AND files were using relative paths
   - Change: `#include "../../src/modules/ModuleName/src/Header.h"` → `#include <ModuleName/Header.h>`
   - Change: `#include "Header.h"` → `#include <ModuleName/Header.h>` (only for moved headers)
   - **Leave relative paths alone for plugin-internal headers in `src/`**

6. **✅ Verify compilation**
   - Build the module and tests
   - Ensure the redirect header works for any legacy code

### Why Keep the Redirect Header?

**Benefits of keeping deprecated private header as a redirect:**

- ✅ **Backward compatible**: Old code that includes `src/Scenario.h` still works
- ✅ **Safe migration**: Gradual transition, not breaking change
- ✅ **Clear intent**: Redirect file explicitly marks path as deprecated
- ✅ **Single source of truth**: All includes ultimately point to public header

**Example of soft migration:**

```cpp
// Old code still compiles:
#include "Scenario.h"  // Redirects to public header through chain

// New code uses public path directly:
#include <ScenarioEngine/Scenario.h>  // Preferred

// Both work, both reference the same class definition
```

## INI Template Generation Helper (ek_generate_ini_template)

- All ini/configuration files for applications and modules must be generated from templates using the `ek_generate_ini_template` CMake macro.
- This macro ensures that all variables (such as working directories, output paths, etc.) are substituted at build time, and that ini files are always up-to-date and consistent.
- **Usage:**
  1. Prepare a template ini file (e.g., `controller.ini.in`) with CMake variables like `@WORKING_DIRECTORY@` and Russian-language comments for all parameters.
  2. In your app/module `CMakeLists.txt`, call:

     ```cmake
     set(WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
     ek_generate_ini_template(controller "${CMAKE_SOURCE_DIR}/runtimes/common/data/controller.ini.in" "${CMAKE_BINARY_DIR}/apps/WatchServiceController" WORKING_DIRECTORY "${WORKING_DIRECTORY}")
     ```

  3. The macro will generate `controller.ini` in the output directory, with all variables replaced by their build-time values.

- See `cmake/README.md` and `docs/build-guide.md` for full documentation and examples.
- **Policy:** All ini templates must be fully documented in Russian, and all new configuration keys must be added to the template and documented accordingly.

## Qt-Specific Guidelines

- **Qt5/Qt6 Compatibility:**
  - Use version macros for compatibility:
    #if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Qt6 code
    #else
    // Qt5 code
    #endif
- **Signal/Slot Syntax:** Prefer new syntax:
  connect(sender, &Sender::signal, receiver, &Receiver::slot);
- **MOC Requirements:** Classes with signals/slots need Q_OBJECT macro.

## CMake Guidelines

- **Use EKiosk CMake Helpers:** Always use the helper functions from cmake/ for all new targets and when refactoring existing CMakeLists.txt files. These include:
  - ek_add_library() from EKLibrary.cmake
  - ek_add_application() from EKApplication.cmake
  - ek_add_plugin() from EKPlugin.cmake
  - ek_add_test() from EKTesting.cmake
  - ek_install_targets(), ek_install_resources() from EKInstall.cmake
  - ek_enable_packaging() from EKPackaging.cmake
  - ek_add_translations() from EKTranslation.cmake
  - ek_enable_static_analysis() from EKStaticAnalysis.cmake

  See cmake/README.md for detailed usage, rationale, and examples for each helper.
  Folder grouping & IDE layout: The ek*add*\* helpers accept an optional `FOLDER` argument (or will compute a sensible default) to group targets in IDE project outlines. Prefer grouping targets as `modules/<module>`, `plugins/<path>`, `apps/<app>`, `tests/<path>` and `thirdparty/<name>` so vendored or example targets do not clutter the root. When working with thirdparty aliases, prefer setting the `FOLDER` on the real target (or use a safe wrapper such as `ek_set_folder_for_target`) rather than on an ALIAS target.
  **Why:**

- Ensures consistent build, test, install, and packaging logic across all apps and modules
- Simplifies CMakeLists.txt files and reduces boilerplate
- Enforces code quality and static analysis standards
- Makes it easy for new contributors to follow project conventions

**How to Refactor:**

- When updating or creating CMakeLists.txt, replace manual target definitions with the appropriate ek\_\* helper function(s).
- Always include the relevant cmake/\*.cmake modules at the top of your CMakeLists.txt.
- Refer to cmake/README.md for copy-paste examples and advanced options.
- If a use case is not covered, extend the helper or document the exception in the code and docs.
- **Qt Version Agnostic:** Use Qt${QT_VERSION_MAJOR}::Module syntax.
- **Platform Checks:** Use if(WIN32), if(UNIX AND NOT APPLE), if(APPLE) for platform-specific code.

# Plugin System Architecture

EKiosk uses a modular plugin system based on Qt plugins with custom factory interfaces. All plugins must implement the `SDK::Plugin::IPluginFactory` interface and be built using the `ek_add_plugin()` CMake helper.

## Plugin Structure

### Directory Layout

```
src/plugins/
├── CategoryName/                    # Plugin category (e.g., GraphicBackends, Payments)
│   └── PluginName/                  # Individual plugin
│       ├── CMakeLists.txt           # Plugin build configuration
│       ├── README.md                # Plugin-specific documentation
│       └── src/                     # Source files
│           ├── PluginFactory.h/.cpp # Qt plugin factory (inherits from SDK::Plugin::PluginFactory)
│           └── PluginImpl.h/.cpp    # Plugin implementation class

tests/plugins/
├── CategoryName/
│   └── PluginName/
│       ├── plugin_test.cpp          # Plugin-specific tests using kernel mocking
│       └── CMakeLists.txt           # Test build configuration
```

**Important:** Plugin tests must be placed in `tests/plugins/` (not `src/plugins/*/tests/`) and use kernel mocking infrastructure from `tests/plugins/common/` (MockKernel, MockLog, PluginTestBase) to ensure proper isolation and avoid Qt test framework cleanup crashes. Tests should achieve 100% coverage of all public methods in plugins and modules.

### Core Interfaces

- **`SDK::Plugin::IPluginFactory`**: Creates plugin instances, provides metadata
- **`SDK::Plugin::IPlugin`**: Base plugin interface with lifecycle methods
- **`SDK::Plugin::IKernel`**: Application kernel providing services to plugins

## Plugin Metadata Source

**Plugin metadata (name, description, author, version) comes from C++ static member variables**, not JSON files. The `PluginFactory` base class provides default implementations that return these static values.

## Creating New Plugins

### 1. Plugin Factory Implementation

```cpp
// PluginFactory.h
class MyPluginFactory : public SDK::Plugin::PluginFactory {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "SDK.Plugin.PluginFactory")  // No FILE parameter needed
    Q_INTERFACES(SDK::Plugin::IPluginFactory)
};

// PluginFactory.cpp
QString SDK::Plugin::PluginFactory::mModuleName = "my_plugin";
QString SDK::Plugin::PluginFactory::mName = "My Plugin";
QString SDK::Plugin::PluginFactory::mDescription = "Description of my plugin";
QString SDK::Plugin::PluginFactory::mAuthor = "Author Name";
QString SDK::Plugin::PluginFactory::mVersion = "1.0";
```

### 2. Plugin Implementation

```cpp
// PluginImpl.h
class MyPlugin : public SDK::Plugin::IPlugin {
public:
    MyPlugin(const QString &instancePath);

    // IPlugin interface implementation
    QString getPluginName() const override;
    QVariantMap getConfiguration() const override;
    void setConfiguration(const QVariantMap &config) override;
    QString getConfigurationName() const override;
    bool saveConfiguration() override;
    bool isReady() const override;
    bool initialize(SDK::Plugin::IKernel *kernel) override;
    bool start() override;
    bool stop() override;
};

// PluginImpl.cpp
namespace {

// Anonymous namespace for internal linkage
SDK::Plugin::IPlugin *CreatePlugin(SDK::Plugin::IEnvironment *factory, const QString &instancePath) {
    return new MyPlugin(instancePath);
}

QVector<SDK::Plugin::SPluginParameter> EnumParameters() {
    return QVector<SDK::Plugin::SPluginParameter>() << SDK::Plugin::SPluginParameter(
        SDK::Plugin::Parameters::Debug, SDK::Plugin::SPluginParameter::Bool, false,
        QT_TRANSLATE_NOOP("MyPluginParameters", "#debug_mode"),
        QT_TRANSLATE_NOOP("MyPluginParameters", "#debug_mode_help"), false);
}

} // namespace

// Register plugin with the system
REGISTER_PLUGIN_WITH_PARAMETERS(
    SDK::Plugin::makePath(SDK::PaymentProcessor::Application,
                          SDK::PaymentProcessor::CComponents::SomeCategory,
                          "MyPlugin"),
    &CreatePlugin, &EnumParameters);

MyPlugin::MyPlugin(const QString &instancePath) : m_instancePath(instancePath) {
    // Constructor implementation
}

// Implement IPlugin methods...
```

### 3. CMake Configuration

```cmake
include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

set(MY_PLUGIN_SOURCES
    src/MyPluginFactory.cpp
    src/MyPluginFactory.h
    src/MyPluginImpl.cpp
    src/MyPluginImpl.h
    # No JSON file needed
)

ek_add_plugin(my_plugin
    FOLDER "plugins/CategoryName"
    SOURCES ${MY_PLUGIN_SOURCES}
    QT_MODULES Core  # Add required Qt modules
    DEPENDS PluginsSDK ek_common
)
```

## Plugin Registration Flow

1. **Static Registration**: `REGISTER_PLUGIN_WITH_PARAMETERS` macro registers the plugin during DLL loading
2. **Qt Plugin Loading**: `Q_PLUGIN_METADATA(IID)` makes the DLL discoverable as a Qt plugin
3. **Factory Instantiation**: Qt creates the factory instance when the plugin is loaded
4. **Plugin Creation**: Factory's `createPlugin()` method instantiates plugin implementation
5. **Metadata Access**: Factory methods return values from C++ static member variables

## Key Design Principles

- **No JSON Files**: Metadata defined in C++ static variables for compile-time safety
- **Anonymous Namespace**: Plugin creation functions have internal linkage to avoid conflicts
- **Factory Pattern**: Clean separation between Qt plugin interface and business logic
- **Registration Macro**: Automatic plugin discovery and registration
- **Testable**: PluginTestBase enables isolated unit testing

## Plugin Testing Framework

All plugins must have comprehensive tests using the mock kernel infrastructure. Tests should achieve 100% coverage of all public methods in plugins and modules, including the complete call chain of classes and dependencies used during plugin execution.

### Coverage Requirements

**100% Coverage Definition:**

- Test all public methods of the plugin factory and implementation classes
- Test all classes instantiated and called by the plugin (e.g., AdPluginFactory → AdPluginImpl → dependent services)
- Cover error paths, edge cases, and integration scenarios
- Use DebugUtils for enhanced debugging when tests fail or for complex scenarios

### Test Structure

```
tests/plugins/
├── common/                          # Shared testing utilities
│   ├── MockObjects.h/.cpp           # Mock implementations
│   ├── PluginTestBase.h/.cpp        # Base test class
│   └── CMakeLists.txt
├── CategoryName/
│   ├── PluginName/
│   │   ├── plugin_test.cpp          # Plugin-specific tests
│   │   └── CMakeLists.txt
```

### Writing Plugin Tests

```cpp
#include "../common/PluginTestBase.h"

class MyPluginTest : public QObject {
    Q_OBJECT

private slots:
    void testPluginLoading();
    void testPluginInitialization();
    void testPluginFunctionality();

private:
    PluginTestBase m_testBase;
};

void MyPluginTest::testPluginLoading() {
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);
    QCOMPARE(factory->getName(), QString("My Plugin"));
}

void MyPluginTest::testPluginInitialization() {
    // Test plugin initialization with mock kernel
    // Verify proper setup and error handling
}

void MyPluginTest::testPluginFunctionality() {
    // Test actual plugin functionality
    // Use mock objects to verify behavior
}
```

### Test CMake Configuration

```cmake
ek_add_test(my_plugin_test
    FOLDER "tests/plugins"
    SOURCES plugin_test.cpp
    DEPENDS PluginTestCommon my_plugin
    QT_MODULES Test Core
)
```

# DebugUtils Module

## Purpose

Provides debugging utilities for call stack dumping, unhandled exception handling and trace logging to help diagnose crashes and runtime issues.

## Features

- **Call Stack Dumping:** Cross-platform stack trace capture using Boost.Stacktrace
- **Unhandled Exception Handling:** Global exception handlers for crash diagnostics
- **Trace Logging:** Function entry/exit logging macros for debugging

## Usage in Tests

When writing tests, use DebugUtils for enhanced debugging capabilities:

```cpp
#include <DebugUtils/DebugUtils.h>

// In test setup or when debugging failures
QStringList stack;
DumpCallstack(stack, nullptr);
// Log or analyze stack for diagnostics

// Enable trace logging for complex test scenarios
ENABLE_TRACE_LOGGER("TestModule");
LOG_TRACE("Test step started");
```

## Platform Support

- **Windows:** Full support with Boost.Stacktrace + WinDbg
- **Linux:** Full support with Boost.Stacktrace + libbacktrace
- **macOS:** Full support with Boost.Stacktrace + libbacktrace

## Testing Guidelines

When writing tests with 100% coverage:

- **Complete Call Chain Testing:** Test not just the plugin factory, but all classes and methods called during plugin execution
- **Dependency Testing:** For plugins like AdPluginFactory that create AdPluginImpl instances, test the public methods and logic of AdPluginImpl and all dependent classes
- **Integration Testing:** Ensure the entire plugin initialization and operation flow is covered, including error paths and edge cases

### Handling Plugin Dependencies in Tests

When plugins depend on external services (settings, database, network, etc.), mock these dependencies to ensure isolated testing:

- **Identify Dependencies:** Check plugin initialization code for services required (e.g., IApplication, ISettingsManager, IDatabase)
- **Create Mock Services:** Add mock implementations to `tests/plugins/common/MockObjects.h` following the pattern of MockKernel/MockLog
- **Initialize Plugins Properly:** After creating a plugin instance, cast to the specific plugin implementation and call `initialize()` with the mock kernel
- **Verify Readiness:** Always test `plugin->isReady()` after proper initialization to ensure all dependencies are satisfied
- **Mock Complex Dependencies:** For plugins that create services internally (like AdService), either mock the service creation or provide minimal implementations that don't require external resources
- **Partial Testing:** If full initialization is not possible due to complex application-level dependencies, test the plugin interface methods that don't require initialization, and document the limitations

Example for testing plugin readiness:

```cpp
void MyPluginTest::testPluginCreation() {
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    SDK::Plugin::IPlugin *plugin = factory->createPlugin("MyPlugin.Instance");
    QVERIFY(plugin != nullptr);

    // For plugins with complex dependencies, test interface without full init
    QVERIFY(!plugin->getPluginName().isEmpty());
    QVERIFY(!plugin->getConfigurationName().isEmpty());

    // If possible, initialize and test readiness
    MyPluginImpl *myPlugin = dynamic_cast<MyPluginImpl *>(plugin);
    if (myPlugin && myPlugin->initialize(&m_testBase.getMockKernel())) {
        QVERIFY(plugin->isReady());
    }

    factory->destroyPlugin(plugin);
}
```

## Plugin Documentation Requirements

### Plugin README.md

Each plugin must have a README.md in its root directory with:

- Plugin purpose and functionality
- **Main class instantiation details** (which file/class gets instantiated when plugin loads)
- Configuration options
- Usage examples
- Dependencies and requirements
- Build instructions
- Testing instructions

### Central Plugin Documentation

Full documentation must be maintained in `docs/plugins/` with:

- Plugin architecture overview
- Integration guides
- API reference
- Configuration reference
- Troubleshooting guides
- Migration notes

### Documentation Checklist

- [ ] Plugin README.md exists and is up-to-date
- [ ] docs/plugins/PluginName.md exists with full documentation
- [ ] API documentation includes all public interfaces
- [ ] Configuration examples provided
- [ ] Integration examples for common use cases
- [ ] Testing documentation includes coverage and CI requirements

## Plugin Categories

### GraphicBackends

- QMLBackend: Qt QML-based graphics rendering
- NativeBackend: Native platform rendering
- WebEngineBackend: Chromium-based rendering

### Payments

- Payment processors and gateways
- Fiscal registration modules
- Receipt generation

### Drivers

- Hardware device drivers
- Communication protocols
- Peripheral interfaces

### NativeScenarios

- Business logic modules
- Workflow automation
- Custom kiosk scenarios

## Plugin Loading and Lifecycle

1. **Discovery**: Qt plugin system scans plugin directories
2. **Registration**: Plugins register with REGISTER_PLUGIN_WITH_PARAMETERS macro
3. **Instantiation**: Factory creates plugin instances on demand
4. **Initialization**: Plugin receives kernel reference and initializes
5. **Operation**: Plugin runs and provides services
6. **Shutdown**: Plugin cleanup and resource release

## Best Practices

- **Error Handling**: Always check return values and handle failures gracefully
- **Logging**: Use kernel-provided logger for all diagnostic output
- **Thread Safety**: Document thread safety guarantees
- **Resource Management**: Properly clean up resources in stop()/destructor
- **Configuration**: Support runtime configuration changes
- **Testing**: Maintain high test coverage with mock kernel
- **Documentation**: Keep README and docs/plugins/ current

# Qt Version Compatibility

All code, CMake, and tests must be written to support both Qt5 and Qt6 where possible.

- Use version-agnostic CMake patterns (e.g., find_package(Qt${QT_VERSION_MAJOR} ...)).
- Prefer Qt APIs and modules available in both versions.
- When Qt version-specific code is required, use CMake or preprocessor checks to handle differences cleanly.

### Platform-Specific Qt Versions

- **Windows 7**: Qt 5.15 LTS (VC toolset 142) - transitional support only
- **Windows 10+ and Linux**: Qt 6.8 LTS

Use platform detection in CMake to select appropriate Qt versions:

```cmake
if(WIN32)
    if(CMAKE_SYSTEM_VERSION VERSION_GREATER_EQUAL "10.0")
        # Windows 10+ - Qt 6
        find_package(Qt6 REQUIRED)
    else()
        # Windows 7 - Qt 5
        find_package(Qt5 REQUIRED)
    endif()
else()
    # Linux - Qt 6
    find_package(Qt6 REQUIRED)
endif()
```

## Conventional Commits & Scopes

All commits must follow the [Conventional Commits](https://www.conventionalcommits.org/) standard.

**Recommended scopes for this project:**

- apps/kiosk, apps/updater, apps/watchdog
- src, include, thirdparty, tests, docs, cmake
- devices, modules, ui, connection, db, other, updater
- build, ci, config, migration

**Examples:**

- feat(apps/kiosk): add new payment module
- fix(devices): resolve printer timeout issue
- docs(getting-started): update setup instructions

Use clear, descriptive scopes to indicate which part of the project is affected.

# Auto-Commit Policy

For all changes that do not require user review (e.g., documentation updates, test scaffolding, non-breaking code changes, or changes that are successfully tested and build without errors), Copilot should auto-commit with a clear, conventional commit message. Only request user confirmation for breaking changes, ambiguous refactors, or when tests/builds fail.

# Module Documentation Guidelines

When creating or updating modules, ensure comprehensive documentation in `docs/modules/`. Each module documentation should follow the canonical, user-facing template described below. Keep implementation-specific structure and contributor notes in `src/modules/<module>/README.md`.

docs/modules/<module>.md (user-facing template - required sections and order):

1. **Title & short summary** (one line)
2. **Purpose** — Why the module exists and what it solves
3. **Quick start / Minimal example** — Copy-paste snippet for getting started
4. **Features** — Short bulleted list
5. **Platform support** — **Required**: include a table with columns `Platform | Status | Notes` covering at minimum Windows, Linux, and macOS (e.g., `Windows | ✅ Full | Supported`, `Linux | 🔬 TODO | Partial or planned`).
6. **Configuration** — Example settings, environment variables or config keys
7. **Usage / API highlights** — Key classes/methods and short examples
8. **Integration** — CMake/linking instructions and integration notes
9. **Testing** — Where tests live and how to run them
10. **Migration notes** — Compatibility, Qt6 notes, or deprecations
11. **Further reading** — Link to `src/modules/<module>/README.md` for implementation details

src/modules/<module>/README.md (implementation, contributor-facing):

- Keep a short pointer to the canonical docs (one line)
- **Structure (implementation)**: file tree with short descriptions for key files
- Implementation notes & design rationale (internal details)
- Build / test hints (CMake quirks, required flags)
- Internal API notes and extension points
- TODOs or migration checklist (if applicable)

Checklist for authors and AI agents (Copilot):

- When moving or creating module docs, follow the docs template above.
- If the original module docs included a file-structure block, move it to `src/modules/<module>/README.md` and replace in `docs/modules/<module>.md` with a short "Implementation & layout" pointer.
- Add an entry for the module in `docs/modules/README.md` (index) and update `docs/getting-started.md` policy notes when applicable.
- Run a markdown linter and link-checker locally or in CI, fix lint issues (headings spacing, fenced code languages, no inline HTML) and broken links before opening a PR.
- **Testing documentation requirement:** The `Testing` section in `docs/modules/<module>.md` must include:
  - Where the tests live (relative path), e.g., `tests/modules/<module>/`
  - The exact commands to run the tests locally (ctest patterns or `cmake --build --target test -R <pattern>`)
  - Any environment variables, external services, or prerequisites (e.g., openssl, hardware) required by the tests
  - Notes about test categorization (unit/integration/slow) and whether tests are flaky or require CI-only execution.
- Use conventional commit messages: `docs(modules): ...` for these changes.

Always include a **Platform Compatibility** section for every module, specifying which platforms are supported and any known limitations.

# EKiosk Qt C++ Project - AI Coding Agent Instructions

## Folder Structure (2026 Modular Redesign)

- **apps/**: Contains all executable applications (e.g., kiosk, updater, watchdog). Each app has its own folder.
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
- Track migration progress in [Migration TODO](../docs/migration-todo.md).

## Best Practices

- Keep apps, shared code, and third-party libraries clearly separated.
- Use CMake targets for each app and library.
- Mirror src/ structure in tests/ for coverage.
- Follow conventional commits and update docs for all major changes.

## Build Guidelines

- **Targeted Builds:** When making code changes, prefer building only the specific target/project affected rather than the entire project, as the project is large. Use `cmake --build . --target <target_name>` for local builds.
- **Local vs Global CMake:** Always try building with local CMake configuration first (`cmake --build .`) unless global CMake reconfiguration is required (e.g., when changing CMakeLists.txt files that affect dependencies).
- **Incremental Testing:** After code changes, test builds on the specific component before committing to avoid long build times on the full project.

## References

- [Getting Started](../docs/getting-started.md)
- [Build Guide](../docs/build-guide.md)
- [Coding Standards](../docs/coding-standards.md)
- [Platform Compatibility](../docs/platform-compatibility.md)
- [Multilanguage Support](../docs/multilanguage.md)
- [Migration Plan & TODO](../docs/migration-todo.md)

---

_For any major change, update the docs and migration-todo list!_

NOTE FOR AI AGENTS: When porting or creating shared application modules (e.g., Base/BasicApplication), add clear code comments (Russian or English as appropriate) for public APIs so maintainers from the original TerminalClient project can understand the ported behavior. Prefer concise doc-comments above public methods with examples if helpful.

## Project-specific Rename Rules (Porting)

When porting code into this repository from other projects, apply the following textual and macro renaming rules so identifiers and macros match EKiosk/HUMO naming:

- **`CYBER` → `HUMO`**: Any project-specific macro, guard, or identifier using the `CYBER` prefix should be renamed to use `HUMO`. Example: `CYBER_SUPPRESS_QT_WARNINGS` → `HUMO_SUPPRESS_QT_WARNINGS`.
- **`TC` → `EC`**: References to `TC` (TerminalClient) used as a short prefix should be renamed to `EC` (EKiosk/EC).
- **From `TCPKiosk` (C#) imports:** The `TCP` prefix found in that project should be renamed to `EC` when porting into this C++ repository.

Notes:

- Apply these renames consistently in code, header guards, CMake variables, and documentation strings when merging or porting code. Prefer mechanical refactoring (search-and-replace) followed by compile and tests.
- If a ported file defines public APIs relied upon by external code, consider adding deprecated aliases or wrapper macros for a transition period rather than an immediate breaking rename.
- Update relevant docs (README, migration notes) when performing these renames so other contributors are aware.
