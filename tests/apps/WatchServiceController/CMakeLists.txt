# Tests for WatchServiceController (tray app)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test Widgets REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

ek_add_test(TestWatchServiceController
    FOLDER "tests/apps/WatchServiceController"
    SOURCES TestWatchServiceController.cpp
    QT_MODULES Test Widgets
    DEPENDS WatchServiceClient Log tray
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/apps/WatchServiceController/src
)
