# Tests for PaymentProcessor module

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

message(STATUS "Configuring PaymentProcessor module tests")

# Directory tests
file(GLOB DIRECTORY_SOURCES Directory/*.cpp Directory/*.h)
ek_add_test(TestDirectory
    FOLDER "tests/modules/PaymentProcessor"
    SOURCES ${DIRECTORY_SOURCES}
    QT_MODULES Test Core
    DEPENDS PPSDK ek_common
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include
)
