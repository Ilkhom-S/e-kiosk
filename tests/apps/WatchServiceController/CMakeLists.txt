# Tests for WatchServiceController (controller app)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test Widgets Network Core REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

# Unit test for WatchServiceController UI
add_executable(TestWatchServiceController TestWatchServiceController.cpp ../../../apps/WatchServiceController/src/WatchServiceController.cpp)
target_link_libraries(TestWatchServiceController PRIVATE Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Widgets BasicApplication WatchServiceClient Log DebugUtils)
target_include_directories(TestWatchServiceController PRIVATE ../../../apps/WatchServiceController/src ${CMAKE_SOURCE_DIR}/include)
set_target_properties(TestWatchServiceController PROPERTIES FOLDER "tests/apps/WatchServiceController")

# Integration test: controller -> watchdog via MessageQueue (TCP)
ek_add_test(TestControllerWatchdogIntegration
    FOLDER "tests/apps/WatchServiceController"
    SOURCES TestControllerWatchdogIntegration.cpp ../../../apps/WatchServiceController/src/WatchServiceController.cpp
    QT_MODULES Test Core Network Widgets
    DEPENDS WatchServiceClient MessageQueue BasicApplication
    INCLUDE_DIRS ../../../apps/WatchServiceController/src ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src/modules/MessageQueue/Tcp/src
)

# End-to-end test: starts real watchdog process and verifies it exits on Exit command
ek_add_test(TestControllerWatchdogE2E
    FOLDER "tests/apps/WatchServiceController"
    SOURCES TestControllerWatchdogE2E.cpp ../../../apps/WatchServiceController/src/WatchServiceController.cpp
    QT_MODULES Test Core Network Widgets
    DEPENDS WatchServiceClient MessageQueue BasicApplication
    INCLUDE_DIRS ../../../apps/WatchServiceController/src ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/src/modules/MessageQueue/Tcp/src
)

