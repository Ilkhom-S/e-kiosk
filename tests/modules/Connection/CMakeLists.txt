# Tests for Connection module

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

message(STATUS "Configuring Connection module tests")

# RAS API tests
file(GLOB RASAPI_SOURCES rasapi/*.cpp rasapi/*.h)
ek_add_test(TestRasApi
    FOLDER "tests/modules/Connection"
    SOURCES ${RASAPI_SOURCES}
    QT_MODULES Test Core
    DEPENDS Connection
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/src/modules/Connection
)
