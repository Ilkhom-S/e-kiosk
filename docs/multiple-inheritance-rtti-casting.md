# Multiple Inheritance & RTTI Casting Issues

## Problem Overview

Non-virtual multiple inheritance combined with RTTI (Runtime Type Information) and `dynamic_cast` can cause **null pointer crashes** and interface casting failures, especially on Qt6 macOS.

### Issue Impact

- **Runtime crash**: `EXC_BAD_ACCESS` when dereferencing null pointer
- **Platform-specific**: More severe on Qt6 macOS, less visible on Qt4/Windows
- **Silent failure**: `dynamic_cast` returns `nullptr` without error
- **Hard to debug**: Pointer appears valid until cast operation

## Root Cause

When a class uses **non-virtual multiple inheritance** from unrelated interfaces:

```cpp
// PROBLEMATIC ❌
class ServiceController : public ICore, public IExternalInterface {
    // Two separate non-virtual inheritance chains
    // ICore* and IExternalInterface* point to different memory offsets
};
```

RTTI's `dynamic_cast` cannot determine the relationship between the two interfaces:

```
Scenario: Convert IExternalInterface* → ICore*

1. PluginService has: ICore* pointer
   ↓ casts to IExternalInterface* for plugin API
   ↓ Plugin receives: IExternalInterface* pointer
2. Plugin tries: dynamic_cast<ICore*>(externalInterface)
   ↓ RTTI checks: "Are ICore and IExternalInterface in same inheritance chain?"
   ↓ RTTI response: "No - they're unrelated interfaces"
   ↓ Result: Returns nullptr (even though same object!)
3. Plugin dereferences nullptr → CRASH
```

### Why Explicit Interface Request Doesn't Help

```cpp
auto *result = aFactory->getInterface(SDK::PaymentProcessor::CInterfaces::ICore);
//                                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//                                     You explicitly ask for ICore, but...

m_Core = dynamic_cast<SDK::PaymentProcessor::ICore*>(result);
//       ^^^^^^^^^^^
//       dynamic_cast doesn't care what you asked for.
//       It only validates what you actually have (IExternalInterface*)
//       → Returns nullptr because RTTI sees them as unrelated
```

**Critical point**: `dynamic_cast` validates pointer type against RTTI metadata, not against what you requested.

## Solution: Use `reinterpret_cast` with `void*` Intermediate

When you **know and control** the object type, use `reinterpret_cast` to bypass RTTI validation:

```cpp
// CORRECT ✅
void *voidPtr = reinterpret_cast<void *>(externalInterface);
ICore *core = reinterpret_cast<ICore *>(voidPtr);
```

**Why this is safe:**

1. ✅ Object type is **known and controlled** (always ServiceController)
2. ✅ Both interfaces are **actually present** in the class definition
3. ✅ Pointer conversion is **deterministic** (compile-time layout)
4. ✅ No **runtime RTTI** involved (no hidden assumptions)

## Affected Code Locations

### Core Infrastructure (Fixed)

- `apps/EKiosk/src/Services/PluginService.cpp` (Line 233)
  - Central interface provider for all plugins
  - Converts `ICore*` → `IExternalInterface*` using reinterpret_cast

- `src/modules/SDK/Plugins/src/PluginFactory.cpp`
  - Plugin factory base class

### Payment Plugins (Fixed)

- `src/plugins/Payments/Humo/src/PaymentFactoryBase.cpp` (Line 48)
  - Humo payment plugin initialization

- `src/plugins/Ad/src/PaymentFactoryBase.cpp`
- `src/plugins/Ad/src/AdRemotePlugin.cpp`
- `src/plugins/Ad/src/AdSourcePlugin.cpp`

### Scenario Backends (Fixed)

- `src/plugins/ScenarioBackends/Uniteller/src/UnitellerBackend.cpp`
- `src/plugins/ScenarioBackends/Uniteller/src/UnitellerChargeProvider.cpp`
- `src/plugins/ScenarioBackends/Uniteller/src/Plugin.cpp`

- `src/plugins/ScenarioBackends/UCS/src/UcsBackend.h`
- `src/plugins/ScenarioBackends/UCS/src/UcsChargeProvider.cpp`
- `src/plugins/ScenarioBackends/UCS/src/Plugin.cpp`

### Native Widgets & UI (Fixed)

- `src/plugins/NativeWidgets/HumoServiceMenu/src/Backend/ServiceMenuBackend.cpp`
- `src/plugins/NativeWidgets/HumoServiceMenu/src/Backend/HumoServiceBackend.cpp`
- `src/plugins/NativeWidgets/HumoServiceMenu/src/AutoEncashment/AutoEncashment.cpp`
- `src/plugins/NativeWidgets/HumoServiceMenu/src/VirtualKeyboard/Keyboard.cpp`
- `src/plugins/NativeWidgets/HumoServiceMenu/src/FirstSetup/FirstSetup.cpp`
- `src/plugins/NativeWidgets/HumoServiceMenu/src/GUI/HumoServiceMenu.cpp`

### Scenario Plugins (Fixed)

- `src/plugins/NativeScenarios/ScreenMaker/src/ScenarioPlugin.h`
- `src/plugins/NativeScenarios/Migrator3000/src/ScenarioPlugin.h`

## Comparison: Safe vs Problematic Multiple Inheritance

### SAFE ✅ (QObject-based)

```cpp
class SafeClass : public QObject, public IMyInterface {
    // QObject uses virtual inheritance
    // RTTI handles pointer conversions automatically
    // dynamic_cast works correctly
};
```

### PROBLEMATIC ❌ (Non-virtual, unrelated interfaces)

```cpp
class ProblematicClass : public ICore, public IExternalInterface {
    // No virtual inheritance
    // Separate, unrelated interface hierarchies
    // RTTI cannot validate relationship
    // dynamic_cast returns nullptr
};
```

## Platform-Specific Behavior

| Platform    | Qt4                                | Qt5         | Qt6            |
| ----------- | ---------------------------------- | ----------- | -------------- |
| Windows 7   | ⚠️ May work (ABI happens to align) | ⚠️ May work | ❌ Crashes     |
| Windows 10+ | ⚠️ May work (ABI happens to align) | ⚠️ May work | ❌ Crashes     |
| Linux       | ⚠️ May work (ABI happens to align) | ⚠️ May work | ❌ Crashes     |
| macOS (Qt6) | N/A                                | N/A         | ❌ **CRASHES** |

**Why Qt6 macOS is most severe:**

- Qt6 uses stricter RTTI validation
- macOS ARM64 has different ABI than x86_64
- Multiple inheritance pointer offsets are non-obvious
- Timing of object destruction more aggressive

## Migration Path (Future)

Use **composition** instead of multiple inheritance:

```cpp
// BETTER DESIGN (Future)
class ServiceController : public ICore {
    // Single inheritance chain

private:
    class ExternalInterfaceAdapter : public IExternalInterface {
        void* getInterface(...) override {
            return m_Parent->getInterface(...);
        }
    private:
        ServiceController* m_Parent;
    };
    ExternalInterfaceAdapter m_ExternalInterface{this};

public:
    IExternalInterface* getExternalInterface() {
        return &m_ExternalInterface;  // No cast needed
    }
};
```

**Benefits of composition approach:**

- ✅ Single inheritance chain (RTTI works correctly)
- ✅ No pointer offset confusion
- ✅ Works consistently across all platforms and Qt versions
- ✅ No need for `reinterpret_cast` workarounds

## Testing & Verification

### Current Fix Verification

```bash
# Build and run with debug output
cmake --build build/macos-qt6 --target ekiosk
./build/macos-qt6/bin/ekiosk.app/Contents/MacOS/ekiosk
```

Should show:

```
PaymentFactoryBase (Humo): POST reinterpret_cast - m_Core = 0x7bb04e900  ✅
PaymentFactoryBase (Humo): crypt service = 0x7bb0052c8                  ✅
PaymentFactoryBase (Humo): network service = 0x7bad48790                ✅
PaymentFactoryBase (Humo): initialization SUCCESS                        ✅
```

### When to Use `reinterpret_cast`

Use ONLY when:

- ✅ Object type is known at compile time
- ✅ You control both interfaces
- ✅ Both interfaces are present in object definition
- ✅ Pointer conversion is deterministic

Do NOT use for:

- ❌ Unknown/external object types
- ❌ Unvalidated pointer sources
- ❌ Polymorphic type hierarchies (use `dynamic_cast`)

## References

- C++14/17 Standard: Reinterpret Cast
- Qt Documentation: Multiple Inheritance with QObject
- Qt6 Migration Guide: RTTI Changes
- CMake Project: ServiceController definition in apps/EKiosk/src/Services/
