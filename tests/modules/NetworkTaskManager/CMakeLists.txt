# Tests for NetworkTaskManager module

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test Network REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

message(STATUS "Configuring NetworkTaskManager module tests")

# NetworkTaskManager tests
file(GLOB NETWORKTASKMANAGER_SOURCES *.cpp *.h)
ek_add_test(TestNetworkTaskManager
    FOLDER "tests/modules/NetworkTaskManager"
    SOURCES ${NETWORKTASKMANAGER_SOURCES}
    QT_MODULES Test Core Network
    DEPENDS NetworkTaskManager BasicApplication
)
