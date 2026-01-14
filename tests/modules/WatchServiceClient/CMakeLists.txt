# WatchServiceClient module tests

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test Core REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

ek_add_test(TestWatchServiceClient
    SOURCES TestWatchServiceClient.cpp
    QT_MODULES Test Core
    DEPENDS WatchServiceClient ek_common
)
