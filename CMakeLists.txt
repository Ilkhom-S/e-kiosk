
cmake_minimum_required(VERSION 3.16)

project(EKioskRoot VERSION 1.0.0 LANGUAGES CXX C)

# Global Qt detection for all modules/apps/tests
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)

# Enable CTest at top-level so tests added in subdirectories are registered
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)
ek_enable_testing()

# Create an INTERFACE target `ek_common` that exports the common include dir
add_library(ek_common INTERFACE)
target_include_directories(ek_common INTERFACE
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)



# Only reference subdirectories for modular apps/libraries
add_subdirectory(apps)
add_subdirectory(tests)
