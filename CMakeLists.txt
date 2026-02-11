cmake_minimum_required(VERSION 3.16)

# 1. Versioning logic
include(scripts/build/cmake/EKVersionConfig.cmake)
ek_configure_version()

# 2. Modern Qt Standard Setup (Qt6 only, safe for Qt5)
# This handles AUTOMOC, AUTOUIC, and AUTORCC automatically for Qt6
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

# Put these BEFORE project() to catch the toolchain initialization
set(VCPKG_VERBOSE OFF CACHE BOOL "")
set(VCPKG_PRINT_POST_INSTALL_MESSAGES OFF CACHE BOOL "")
set(CMAKE_MESSAGE_LOG_LEVEL NOTICE)
# This silences the "package provides CMake targets" usage messages
set(VCPKG_INSTALL_OPTIONS "--no-print-usage" CACHE STRING "")

project(EKioskRoot VERSION 1.0.0 LANGUAGES CXX C)

if(APPLE)
    add_link_options("-Wl,-no_warn_duplicate_libraries")
    enable_language(OBJCXX)
endif()

# 3. Global Qt Detection
# Finds whichever version is available (Qt6 preferred)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Core Widgets Gui Network WebSockets SerialPort Sql Test
    Quick Concurrent Xml LinguistTools QuickWidgets Qml PrintSupport
)

if(QT_VERSION_MAJOR EQUAL 6)
    # This replaces the need for include(Qt6)
    # and sets up AUTOMOC/AUTOUIC correctly.
    set(QT_STANDARD_PROJECT_SETUP ON)
endif()

# 4. Path Configurations (Cleaned up)
if(DEFINED ENV{EK_INCLUDES_DIR})
    set(EK_INCLUDES_DIR $ENV{EK_INCLUDES_DIR} CACHE PATH "Path to public include dir")
else()
    set(EK_INCLUDES_DIR "${CMAKE_SOURCE_DIR}/include" CACHE PATH "Path to public include dir")
endif()

# Build output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
if(DEFINED ENV{EK_BIN_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "$ENV{EK_BIN_DIR}")
endif()

# 5. Thirdparty & Dependencies
# Note: SingleApplication include path is now handled INSIDE thirdparty/CMakeLists.txt
if(EXISTS "${CMAKE_SOURCE_DIR}/thirdparty/CMakeLists.txt")
    add_subdirectory(thirdparty)
endif()

# 6. Boost Configuration (Multiplatform / vcpkg compatible)
# Removed hardcoded 'arm64-osx' path
find_package(Boost REQUIRED)
add_library(ek_boost INTERFACE)
target_link_libraries(ek_boost INTERFACE Boost::boost)

# 7. Common Interface Target
add_library(ek_common INTERFACE)
target_include_directories(ek_common INTERFACE
    $<BUILD_INTERFACE:${EK_INCLUDES_DIR}>
    $<INSTALL_INTERFACE:include>
)

# 8. Feature Options
option(EKIOSK_ENABLE_FR "Enable FR (Fiscal Register) support" OFF)
if(NOT EKIOSK_ENABLE_FR)
    message(STATUS "FR support is disabled (deprecated)")
endif()

option(BUILD_DEPRECATED "Build deprecated code (for reference only)" OFF)
if(BUILD_DEPRECATED)
    message(STATUS "Building deprecated code")
else()
    message(STATUS "Skipping deprecated code (use -DBUILD_DEPRECATED=ON to include)")
endif()

# 9. Additional Logic Modules
include(${CMAKE_SOURCE_DIR}/cmake/EKTestingSetup.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/EKIniTemplate.cmake)

# 10. Project Subdirectories
add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(tests)

# 11. Deprecated Code (optional)
if(BUILD_DEPRECATED)
    add_subdirectory(deprecated)
endif()
