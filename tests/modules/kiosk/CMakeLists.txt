# Tests for apps/kiosk module

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

message(STATUS "Configuring kiosk module tests")

file(GLOB EXAMPLE_SOURCES Example/*.cpp Example/*.h)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Example")
    ek_add_test(TestExample
        FOLDER "tests/kiosk"
        SOURCES ${EXAMPLE_SOURCES}
        QT_MODULES Test
        DEPENDS ek_common
    )
endif()

ek_add_test(TestStringConversions
    FOLDER "tests/kiosk"
    SOURCES TestStringConversions.cpp
    QT_MODULES Test Core
    DEPENDS ek_common
)

ek_add_test(TestPrinterConversion
    FOLDER "tests/kiosk"
    SOURCES TestPrinterConversion.cpp
    QT_MODULES Test Core
    DEPENDS ek_common
)

ek_add_test(TestConnectPrivilege
    FOLDER "tests/kiosk"
    SOURCES TestConnectPrivilege.cpp
    QT_MODULES Test Core
    DEPENDS ek_common
)
