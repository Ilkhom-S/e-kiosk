
# EKiosk Tests CMake



cmake_minimum_required(VERSION 3.16)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

message(STATUS "Configuring tests for EKiosk")

# Test categories
add_subdirectory(unit)
# add_subdirectory(integration)  # Uncomment when ready

# Enable testing
ek_enable_testing()
