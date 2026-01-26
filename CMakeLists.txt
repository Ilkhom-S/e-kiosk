


# Include EKiosk version configuration logic
include(scripts/build/cmake/EKVersionConfig.cmake)

# Generate version files during configure step (so they always exist for IDEs and build)
ek_configure_version()


cmake_minimum_required(VERSION 3.16)

# Ensure SingleApplication include path is available globally on non-Windows platforms
if(NOT WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/thirdparty/SingleApplication)
endif()

# When supported, prefer the modern behavior for Boost package lookup and
# avoid the developer warning about the removed FindBoost module.
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

project(EKioskRoot VERSION 1.0.0 LANGUAGES CXX C)

# Allow overriding the public include directory via the EK_INCLUDES_DIR environment variable
# Final fallback is ${CMAKE_SOURCE_DIR}/include to keep local builds working.
if(DEFINED ENV{EK_INCLUDES_DIR})
    set(EK_INCLUDES_DIR $ENV{EK_INCLUDES_DIR} CACHE PATH "Path to public include dir (overridden by EK_INCLUDES_DIR env)")
elseif(NOT DEFINED EK_INCLUDES_DIR)
    set(EK_INCLUDES_DIR ${CMAKE_SOURCE_DIR}/include CACHE PATH "Path to public include dir")
endif()
message(STATUS "Using EK includes dir: ${EK_INCLUDES_DIR}")

# Set build output directory for all configs from EK_BIN_DIR if set, else use default
if(DEFINED ENV{EK_BIN_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$ENV{EK_BIN_DIR}")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()
foreach(_cfg IN ITEMS Debug Release RelWithDebInfo MinSizeRel)
    string(TOUPPER "${_cfg}" _CFG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${_CFG_UPPER} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endforeach()

# Global Qt detection for all modules/apps/tests
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)

# Load the actual Qt package with required components
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Gui Network WebSockets SerialPort Sql Quick Concurrent Xml LinguistTools QuickWidgets Qml)

# Find Boost (installed via vcpkg) - header-only usage only
# find_package(Boost REQUIRED)

# SingleApplication setup for freestanding mode
set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")

# Thirdparty libraries are configured in thirdparty/CMakeLists.txt
# This provides clean EK::* targets for all dependencies
if(EXISTS "${CMAKE_SOURCE_DIR}/thirdparty/CMakeLists.txt")
    add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty ${CMAKE_BINARY_DIR}/thirdparty)
endif()

# Testing and coverage configuration
include(${CMAKE_SOURCE_DIR}/cmake/EKTestingSetup.cmake)

# INI template generation helper (ek_generate_ini_template)
include(${CMAKE_SOURCE_DIR}/cmake/EKIniTemplate.cmake)

# FR (Fiscal Register) support - deprecated, disabled by default
option(EKIOSK_ENABLE_FR "Enable FR (Fiscal Register) support" OFF)
if(NOT EKIOSK_ENABLE_FR)
    message(STATUS "FR (Fiscal Register) support is disabled (deprecated)")
endif()


# Create an INTERFACE target `ek_common` that exports the common include dir
add_library(ek_common INTERFACE)
# Use the overridable EK_INCLUDES_DIR so downstream builds can point to a different include tree
target_include_directories(ek_common INTERFACE
    $<BUILD_INTERFACE:${EK_INCLUDES_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Create an INTERFACE target for Boost includes
add_library(ek_boost INTERFACE)
target_include_directories(ek_boost INTERFACE ${CMAKE_SOURCE_DIR}/vcpkg_installed/arm64-osx/include)

# Thirdparty libraries are configured in thirdparty/CMakeLists.txt
# Only reference subdirectories for modular apps/libraries
add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(tests)


