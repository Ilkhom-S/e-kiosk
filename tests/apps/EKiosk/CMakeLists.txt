# Tests for apps/EKiosk
message(STATUS "Configuring EKiosk app tests")

include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

if(WIN32)
    ek_add_test(TestPrinterConversion
        SOURCES TestPrinterConversion.cpp
        FOLDER "tests/apps/EKiosk"
    )

    ek_add_test(TestStringConversions
        SOURCES TestStringConversions.cpp
        FOLDER "tests/apps/EKiosk"
    )

    ek_add_test(TestConnectPrivilege
        SOURCES TestConnectPrivilege.cpp
        FOLDER "tests/apps/EKiosk"
    )

    ek_add_test(TestBaseApplication
        SOURCES TestBaseApplication.cpp
        FOLDER "tests/apps/EKiosk"
    )
endif()

add_subdirectory(Example)
