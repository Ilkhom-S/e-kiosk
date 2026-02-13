# Qt6 Container Iterator Safety

## Problem Overview

In Qt6, constructing containers (QSet, QList, QVector) with iterators from temporary containers causes **undefined behavior** and runtime crashes like `EXC_BAD_ACCESS`.

### Issue Impact

- **Runtime crash**: Program terminates without error message
- **Platform-specific**: More common on macOS
- **Hard to debug**: No exception or stack trace
- **Silent failure**: App just shuts down

## Root Cause

When calling a method like `.keys()` or `.values()` directly in constructor arguments, multiple temporary objects are created:

```cpp
// WRONG ❌
QSet<QString> result(map.keys().begin(), map.keys().end());
//                     ^first temp      ^second temp (different object!)
```

Timeline:

1. `.keys()` called (first) → creates temporary `QList`
2. `.end()` called → creates NEW temporary `QList`
3. `.begin()` and `.end()` iterators point to DIFFERENT temporary objects
4. Temporaries destroyed after the expression
5. QSet constructor receives dangling/invalid iterators → crash

## Solution: Store First, Use Second

**Always store the container in a variable before using its iterators:**

```cpp
// CORRECT ✅
const auto keys = map.keys();           // Store in variable
QSet<QString> result(keys.cbegin(),     // Use |
                     keys.cend());      // same container
```

### Why This Works

- Container stored in named variable (persistent)
- `.cbegin()` and `.cend()` called on same object
- Iterators valid for constructor lifetime
- No dangling pointers

### Use `cbegin()` / `cend()`

- **Const iterators** are safer
- Required for read-only operations (which constructors are)
- Qt6 best practice

## Examples

### QSet from Map Keys

```cpp
// WRONG
TParameterSet params(paramMap.keys().begin(), paramMap.keys().end());

// CORRECT
const auto keys = paramMap.keys();
TParameterSet params(keys.cbegin(), keys.cend());
```

### QSet from Map Values

```cpp
// WRONG
TStatusCodes statuses(statusMap.values().begin(), statusMap.values().end());

// CORRECT
const auto values = statusMap.values();
TStatusCodes statuses(values.cbegin(), values.cend());
```

### In Foreach Loops

```cpp
// WRONG
foreach (const Item &item, QSet<Item>(list.begin(), list.end())) {
    // ...
}

// CORRECT
const auto items = list;
foreach (const Item &item, QSet<Item>(items.cbegin(), items.cend())) {
    // ...
}
```

### In Return Statements

```cpp
// WRONG
return QStringList(keys.begin(), keys.end());

// CORRECT
const auto keysTemp = keys;  // Or use intermediate variable
return QStringList(keysTemp.cbegin(), keysTemp.cend());
```

## Affected Code Patterns

This issue applies to any container constructor taking `.begin()/.end()`:

| Container          | Unsafe              | Safe                   |
| ------------------ | ------------------- | ---------------------- |
| `QSet<T>(...)`     | `.keys()/.values()` | store + `c*begin/cend` |
| `QList<T>(...)`    | `.keys()/.values()` | store + `c*begin/cend` |
| `QVector<T>(...)`  | `.keys()/.values()` | store + `c*begin/cend` |
| Range constructors | Temporary iterators | Named variable first   |

## Fixed Instances

During Feb 2026 session, fixed 9 unsafe patterns:

1. **CashAcceptorBase.cpp** (3 instances)
   - Line 810: `lastStatuses.keys()`
   - Line 924: `lastStatusHistory.statuses.keys()`
   - Line 947: `statusBuffer.keys()`

2. **PaymentService.cpp** (1 instance)
   - Line 142: Two temporary `.keys()` calls

3. **CashDispenserManager.cpp** (1 instance)
   - Line 218: `m_Dispensers.keys()`

4. **PrintingCommands.cpp** (1 instance)
   - Line 218: `aParameters.keys()`

5. **IntegratedDrivers.cpp** (1 instance)
   - Line 75: `pathsByModel.values()`

6. **ServiceController.cpp** (1 instance)
   - Line 407: `m_RegisteredServices.values()`

7. **DeviceManager.cpp** (1 instance)
   - Line 571: `m_RDSystem_Names.keys()`

8. **DealerSettings.cpp** (1 instance)
   - Line 707: `m_ProvidersProcessingIndex.keys()`

## Prevention for Agents

**Code generation rules:**

- ❌ **Never** construct containers directly from `.keys()/.values()`
- ✅ **Always** store in temporary variable first
- ✅ Use `cbegin()` / `cend()` for const safety
- ✅ Add comment explaining the safety need

Example template:

```cpp
// Qt6: Store keys first to avoid dangling iterator danger
const auto keys = container.keys();
QSet<MyType> result(keys.cbegin(), keys.cend());
```

## Testing

To verify a fix:

1. Build: `cmake --build build/macos-qt6 --target ekiosk`
2. Run: Check logs for the component
3. If silent crash disappears → iterator safety improved ✅
4. If crash persists → verify all code paths

## See Also

- Qt6 migration: [docs/qt-migration-plan.md](qt-migration-plan.md)
- Qt6 documentation: <https://doc.qt.io/qt-6/containers.html>
