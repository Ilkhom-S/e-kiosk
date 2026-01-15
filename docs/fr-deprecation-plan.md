# FR (Fiscal Register) Deprecation Migration Plan

## Overview

The FR (Fiscal Register) module, including its plugins, drivers, and related devices, is not needed for the EKiosk project. This document outlines a 2-phase migration plan to deprecate and remove all FR-related code.

## Current FR Components

The following FR-related components exist in the codebase:

- **Plugins:**

  - `src/plugins/Drivers/FR/` - Fiscal register driver plugin
  - `src/plugins/ScenarioBackends/UCS/` - UCS scenario backend (may depend on FR)
  - `src/plugins/ScenarioBackends/Uniteller/` - Uniteller scenario backend (may depend on FR)

- **Modules:**

  - `src/modules/Hardware/FR/` - Hardware FR module
  - `include/Hardware/FR/` - FR headers

- **Dependencies:**
  - HardwareCommon
  - HardwareFR
  - DriversSDK
  - HardwareIOPorts
  - EK::LibUSB (used by FR plugin)

## Migration Phases

### Phase 1: Build Exclusion (Completed)

**Goal:** Exclude FR components from the build system while keeping code intact for future removal.

**Tasks:**

- [x] Modify root CMakeLists.txt to conditionally exclude FR plugin
- [x] Update src/plugins/Drivers/CMakeLists.txt to skip FR
- [x] Update src/modules/Hardware/CMakeLists.txt to conditionally build HardwareFR and HardwareProtocols FR components
- [x] Add EKIOSK_ENABLE_FR CMake option (default OFF)
- [x] Update documentation to mark FR as deprecated
- [x] Test build without FR components - **SUCCESSFUL**

**Implementation:**

- Added `EKIOSK_ENABLE_FR` CMake option (default `OFF`) in root CMakeLists.txt
- Modified `src/plugins/Drivers/CMakeLists.txt` to conditionally include FR plugin
- Modified `src/modules/Hardware/CMakeLists.txt` to conditionally build HardwareFR library and FR protocols in HardwareProtocols
- Tested successful build of EKiosk application without FR components

### Phase 2: Complete Removal (Future)

**Goal:** Completely remove all FR-related code, headers, and logic.

**Tasks:**

- [ ] Remove FR plugin directory (`src/plugins/Drivers/FR/`)
- [ ] Remove FR module directory (`src/modules/Hardware/FR/`)
- [ ] Remove FR headers (`include/Hardware/FR/`)
- [ ] Remove FR-dependent scenario backends if they only serve FR
- [ ] Clean up any FR references in other modules
- [ ] Update CMakeLists.txt files to remove FR references
- [ ] Update documentation to remove FR references
- [ ] Test build and functionality without FR code

## Implementation Details

### Phase 1 Implementation

Add CMake option to control FR inclusion:

```cmake
option(EKIOSK_ENABLE_FR "Enable FR (Fiscal Register) support" OFF)
```

Conditionally include FR in CMakeLists.txt:

```cmake
if(EKIOSK_ENABLE_FR)
    add_subdirectory(FR)
endif()
```

### Dependencies Impact

- **HardwareCommon:** May be used by other hardware modules - keep
- **HardwareFR:** FR-specific - can be removed in Phase 2
- **DriversSDK:** May be used by other drivers - keep
- **HardwareIOPorts:** May be used by other hardware - keep
- **EK::LibUSB:** Used by FR but may be needed by other components - keep

## Timeline

- **Phase 1:** Immediate - Complete build exclusion
- **Phase 2:** Future - Complete code removal when convenient

## Notes

- FR code is marked as deprecated but kept intact for gradual removal
- Build system changes are minimal and reversible
- No functional impact on remaining EKiosk features
- FR removal enables cleaner codebase and faster builds
