# EKiosk Tests CMake
cmake_minimum_required(VERSION 3.16)

# 1. Component check: Only find Test if it hasn't been found in root already
if(NOT Qt${QT_VERSION_MAJOR}Test_FOUND)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)
endif()

# 2. Inherit the Move Semantics & Warnings
# This ensures that 'pass by value and use std::move' logic is enforced in tests too
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 3. Include testing logic
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

message(STATUS "Configuring tests for EKiosk (Qt ${QT_VERSION_MAJOR})")

# 4. Global Test Configuration
# Enable testing before adding test subdirectories so tests register correctly
ek_enable_testing()

# 5. Test categories
# Use a helper function for consistent folder organization in IDEs (Xcode/VS)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(modules)
add_subdirectory(plugins)
add_subdirectory(scripts)
add_subdirectory(apps)

# add_subdirectory(integration)  # Uncomment when ready
