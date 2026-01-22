# Tests for DebugUtils module

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

message(STATUS "Configuring DebugUtils module tests")

# Main DebugUtils tests
ek_add_test(TestDebugUtils
    FOLDER "tests/modules/DebugUtils"
    SOURCES TestDebugUtils.cpp
    QT_MODULES Test Core
    DEPENDS DebugUtils
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include
)

# Stack trace performance tests
ek_add_test(TestDebugUtilsPerformance
    FOLDER "tests/modules/DebugUtils"
    SOURCES TestDebugUtilsPerformance.cpp
    QT_MODULES Test Core
    DEPENDS DebugUtils
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include
)

# Cross-platform compatibility tests
ek_add_test(TestDebugUtilsCrossPlatform
    FOLDER "tests/modules/DebugUtils"
    SOURCES TestDebugUtilsCrossPlatform.cpp
    QT_MODULES Test Core
    DEPENDS DebugUtils
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include
)

# Add _GNU_SOURCE for Boost.Stacktrace on Unix-like systems
if(UNIX AND NOT APPLE)
    target_compile_definitions(TestDebugUtilsCrossPlatform PRIVATE _GNU_SOURCE)
elseif(APPLE)
    target_compile_definitions(TestDebugUtilsCrossPlatform PRIVATE _GNU_SOURCE)
endif()
