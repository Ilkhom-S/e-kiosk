# WatchServiceClient module tests

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test Core REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

ek_add_test(TestWatchServiceClient
    FOLDER "tests/modules/WatchServiceClient"
    SOURCES TestWatchServiceClient.cpp mock_basicapplication.cpp ${CMAKE_SOURCE_DIR}/src/modules/Common/application/src/BasicApplication.cpp
    QT_MODULES Test Core
    DEPENDS WatchServiceClient ek_common SingleApplication
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/thirdparty/SingleApplication
)
