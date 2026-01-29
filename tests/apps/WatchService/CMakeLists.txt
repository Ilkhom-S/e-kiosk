# Tests for WatchService (guard app)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

message(STATUS "Configuring WatchService app tests")

# Test for CpuSpeed function
ek_add_test(TestCpuSpeed
    FOLDER "tests/apps/WatchService"
    SOURCES TestCpuSpeed.cpp ../../../apps/WatchService/src/CpuSpeed.cpp
    QT_MODULES Test Core
    DEPENDS
    INCLUDE_DIRS ../../../apps/WatchService/src ${CMAKE_SOURCE_DIR}/include
)
