# Tests for WatchServiceController (tray app)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test Widgets REQUIRED)

add_executable(TestWatchServiceController TestWatchServiceController.cpp ../../../apps/WatchServiceController/src/WatchServiceController.cpp)
target_link_libraries(TestWatchServiceController PRIVATE Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Widgets BasicApplication WatchServiceClient Log DebugUtils)
target_include_directories(TestWatchServiceController PRIVATE ../../../apps/WatchServiceController/src ${CMAKE_SOURCE_DIR}/include)
set_target_properties(TestWatchServiceController PROPERTIES FOLDER "tests/apps/WatchServiceController")
