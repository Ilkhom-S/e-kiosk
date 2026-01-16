
cmake_minimum_required(VERSION 3.16)

# When supported, prefer the modern behavior for Boost package lookup and
# avoid the developer warning about the removed FindBoost module.
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

project(EKioskRoot VERSION 1.0.0 LANGUAGES CXX C)

# Global Qt detection for all modules/apps/tests
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)

# Load the actual Qt package with required components
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Gui Network WebSockets SerialPort Sql Quick Concurrent Xml)

# Find Boost (installed via vcpkg)
find_package(Boost REQUIRED)

# SingleApplication setup for freestanding mode
set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")

# Thirdparty libraries are configured in thirdparty/CMakeLists.txt
# This provides clean EK::* targets for all dependencies
if(EXISTS "${CMAKE_SOURCE_DIR}/thirdparty/CMakeLists.txt")
    add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty ${CMAKE_BINARY_DIR}/thirdparty)
endif()

# Testing and coverage configuration
include(${CMAKE_SOURCE_DIR}/cmake/EKTestingSetup.cmake)

# FR (Fiscal Register) support - deprecated, disabled by default
option(EKIOSK_ENABLE_FR "Enable FR (Fiscal Register) support" OFF)
if(NOT EKIOSK_ENABLE_FR)
    message(STATUS "FR (Fiscal Register) support is disabled (deprecated)")
endif()

# Create an INTERFACE target `ek_common` that exports the common include dir
add_library(ek_common INTERFACE)
target_include_directories(ek_common INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Create an INTERFACE target for Boost includes
add_library(ek_boost INTERFACE)
target_include_directories(ek_boost INTERFACE ${Boost_INCLUDE_DIRS})

# Thirdparty libraries are configured in thirdparty/CMakeLists.txt
# Only reference subdirectories for modular apps/libraries
add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(tests)


