# **EKiosk Code Agent Instructions**

## **🎯 Core Rules**

- **Brief responses only** - confirm task completion
- **Follow project patterns** - match existing code style
- **Use clang-format/clang-tidy** settings from `.clang-format`, `.clang-tidy`

## **📂 Project Structure**

```
apps/           → Executables
src/            → Shared libs/modules
include/        → Public headers
tests/          → Mirror src/ structure
```

## **⚙️ Qt Version Logic**

```cpp
// Windows 7 → Qt5, Others → Qt6
#ifdef WIN32
    #if CMAKE_SYSTEM_VERSION >= "10.0"
        Qt6::Module
    #else
        Qt5::Module
    #endif
#else
    Qt6::Module  // Linux
#endif
```

## **🔧 Code Generation Rules**

### **Naming**

- Classes: `PascalCase`
- Methods: `camelCase`
- Members: `mCamelCase`
- Interfaces: `IInterface`

### **Headers**

```cpp
#pragma once                    // Use this
/* @file Russian description */ // Required file header
```

### **CMake**

```cmake
include(${CMAKE_SOURCE_DIR}/cmake/EK*.cmake)
ek_add_library()     # Always use EK helpers
Qt${QT_VERSION_MAJOR}::Module  # Version-agnostic Qt
```

### **Plugin Structure**

```cpp
// Required: Factory + Impl classes
// Metadata in C++, no JSON
// Use REGISTER_PLUGIN_WITH_PARAMETERS()
```

## **⚠️ Qt6 Container Iterator Safety**

### **Critical Issue: Temporary Container Iterators**

**UNSAFE PATTERN** ❌ (Causes EXC_BAD_ACCESS on macOS):

```cpp
// WRONG - Two different temperary objects created
QSet<QString> result(map.keys().begin(), map.keys().end());
QSet<int> statusSet(deviceData.values().begin(), deviceData.values().end());

// Also WRONG in QSet/QList constructors
foreach (const Item &item, QSet<Item>(list.begin(), list.end())) { }
```

**Why it crashes:**

- `.keys()` creates a temporary `QList`
- `.begin()` and `.end()` from different temporary calls = iterator from destroyed objects
- Constructor receives dangling/invalid iterators → undefined behavior

**SAFE PATTERN** ✅:

```cpp
// RIGHT - Store container first, then use its iterators
const auto keys = map.keys();
QSet<QString> result(keys.cbegin(), keys.cend());

const auto values = deviceData.values();
QSet<int> statusSet(values.cbegin(), values.cend());

// For foreach loops
const auto items = list;
foreach (const Item &item, QSet<Item>(items.cbegin(), items.cend())) { }
```

**Applies to:**

- `QSet<T>(container.keys()...)`
- `QSet<T>(container.values()...)`
- `QList<T>(container.keys()...)`
- `QVector<T>(container.values()...)`
- Any container constructor taking `.begin()/.end()` from method calls

**Always use `cbegin()/cend()`** (const iterators) for safety with Qt6.

👉 **See full documentation:** [docs/qt6-iterator-safety.md](docs/qt6-iterator-safety.md)

## **⚠️ Multiple Inheritance & RTTI Casting Issues**

### **Critical Issue: Non-Virtual Multiple Inheritance with `dynamic_cast`**

**UNSAFE PATTERN** ❌ (Causes EXC_BAD_ACCESS null pointer crash):

```cpp
// WRONG - Non-virtual multiple inheritance
class ServiceController : public ICore, public IExternalInterface {
    // Two separate inheritance chains → RTTI can't bridge them
};

// Plugin code tries:
ICore *core = dynamic_cast<ICore*>(externalInterface);
// Result: Returns nullptr (even though same object!)
// Plugin dereferences: → CRASH
```

**Why it fails:**

- `dynamic_cast` uses RTTI to validate pointer type relationship
- `ICore` and `IExternalInterface` are in separate, unrelated inheritance chains
- RTTI cannot determine they're the same object → returns nullptr
- Explicit interface request doesn't help (`dynamic_cast` validates what you _have_, not what you _asked_ for)

**SAFE PATTERN** ✅:

```cpp
// RIGHT - Use reinterpret_cast with void* intermediate
void *voidPtr = reinterpret_cast<void *>(externalInterface);
ICore *core = reinterpret_cast<ICore *>(voidPtr);

// Why this is safe:
// - Object type is known and controlled (always ServiceController)
// - Both interfaces present in class definition
// - Pointer conversion is deterministic (compile-time layout)
// - Works across Qt4, Qt5, Qt6, all platforms (Windows 7 to Qt6)
```

**Applies to (all fixed):**

- `PluginService::getInterface()` - Core interface provider
- `PaymentFactoryBase` - Humo, Ad plugins
- `ScenarioBackends` - Uniteller, UCS
- `NativeWidgets` - HumoServiceMenu, AutoEncashment, etc.
- `NativeScenarios` - ScreenMaker, Migrator3000

👉 **See full documentation:** [docs/multiple-inheritance-rtti-casting.md](docs/multiple-inheritance-rtti-casting.md)

## **✅ Response Format**

- ✅ Task completed
- 📝 Files modified: `list`
- ⚠️ Issues: `brief description`

---

**Is this the format you want?** Focused on **code editing instructions** with **minimal explanations**.

- ✅ Task completed
- 📝 Files modified: `list`
- ⚠️ Issues: `brief description`

---
