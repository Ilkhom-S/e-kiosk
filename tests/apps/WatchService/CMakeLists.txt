# Tests for WatchService (watchdog app)

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

# Determine which process enumerator implementation to use for tests
if(WIN32)
    set(PROCESS_ENUMERATOR_SRC ../../../apps/WatchService/src/win_processenumerator.cpp)
elseif(APPLE)
    set(PROCESS_ENUMERATOR_SRC ../../../apps/WatchService/src/mac_processenumerator.cpp)
else()
    set(PROCESS_ENUMERATOR_SRC ../../../apps/WatchService/src/linux_processenumerator.cpp)
endif()

# Test for ProcessEnumerator class
ek_add_test(TestProcessEnumerator
    FOLDER "tests/apps/WatchService"
    SOURCES TestProcessEnumerator.cpp ../../../apps/WatchService/src/processenumerator.h ${PROCESS_ENUMERATOR_SRC}
    QT_MODULES Test Core
    DEPENDS
    INCLUDE_DIRS ../../../apps/WatchService/src ${CMAKE_SOURCE_DIR}/include
)
