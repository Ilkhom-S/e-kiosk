
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

# SingleApplication setup for freestanding mode
set(QAPPLICATION_CLASS QApplication CACHE STRING "Inheritance class for SingleApplication")

# If vendored thirdparty CMake helper exists, add it so vendored libraries
# (like SingleApplication) are available as targets.
if(EXISTS "${CMAKE_SOURCE_DIR}/thirdparty/CMakeLists.txt")
    add_subdirectory(${CMAKE_SOURCE_DIR}/thirdparty ${CMAKE_BINARY_DIR}/thirdparty)
endif()

# Prefer the vendored target if present; otherwise fall back to find_package
if(TARGET SingleApplication::SingleApplication)
    message(STATUS "Using vendored SingleApplication target")
else()
    find_package(SingleApplication REQUIRED)
endif()

# Enable CTest at top-level so tests added in subdirectories are registered
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)
ek_enable_testing()

# Optional: enable coverage build and reporting (Linux/Clang/GNU)
option(ENABLE_COVERAGE "Enable coverage build and reporting" OFF)
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        message(STATUS "Coverage enabled: adding --coverage flags")
        set(COVERAGE_FLAGS "--coverage -O0 -g")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_FLAGS}")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_FLAGS}")
    else()
        message(WARNING "Coverage build requested but compiler does not support gcov/llvm-cov flags")
    endif()
endif()

# Coverage target: generate lcov info and HTML report
if(ENABLE_COVERAGE AND (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang"))
    find_program(LCOV_EXEC lcov)
    find_program(GENHTML_EXEC genhtml)
    if(NOT LCOV_EXEC)
        message(WARNING "lcov not found: coverage target will fail without lcov installed")
    endif()
    if(NOT GENHTML_EXEC)
        message(WARNING "genhtml not found: coverage target will fail without genhtml installed")
    endif()
    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target test -- -j
        COMMAND ${LCOV_EXEC} --directory ${CMAKE_BINARY_DIR} --capture --output-file ${CMAKE_BINARY_DIR}/coverage.info
        COMMAND ${LCOV_EXEC} --remove ${CMAKE_BINARY_DIR}/coverage.info '/usr/*' --output-file ${CMAKE_BINARY_DIR}/coverage.info
        COMMAND ${GENHTML_EXEC} ${CMAKE_BINARY_DIR}/coverage.info --output-directory ${CMAKE_BINARY_DIR}/coverage-report
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating coverage report (requires lcov/genhtml)"
        VERBATIM
    )
endif()

# Create an INTERFACE target `ek_common` that exports the common include dir
add_library(ek_common INTERFACE)
target_include_directories(ek_common INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# DelayImpHlp
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/DelayImpHlp)
    if(NOT TARGET DelayImpHlp)
        add_library(DelayImpHlp INTERFACE)
        target_include_directories(DelayImpHlp INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/DelayImpHlp)
    endif()
else()
    if(NOT TARGET DelayImpHlp)
        add_library(DelayImpHlp INTERFACE)
    endif()
endif()


# Only reference subdirectories for modular apps/libraries
add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(tests)
