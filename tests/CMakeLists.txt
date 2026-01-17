
# EKiosk Tests CMake



cmake_minimum_required(VERSION 3.16)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

message(STATUS "Configuring tests for EKiosk")

# Enable testing before adding test subdirectories so tests register correctly
ek_enable_testing()

# Test categories
add_subdirectory(unit)
add_subdirectory(modules)
add_subdirectory(plugins)
add_subdirectory(scripts)

# add_subdirectory(integration)  # Uncomment when ready
