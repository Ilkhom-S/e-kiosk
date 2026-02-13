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
