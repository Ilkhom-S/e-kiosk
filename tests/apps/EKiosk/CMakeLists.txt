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

# Database resource validation test
# Verifies that Database.qrc resources are properly compiled
# Helps diagnose Qt4→Qt5/Qt6 migration issues with resource compilation
#
# NOTE: This test needs access to Database.qrc resources, so it compiles
# the Database.qrc file directly rather than linking to ekiosk
ek_add_test(ekiosk_database_validation
    SOURCES
    database_validation_test.cpp
    ${CMAKE_SOURCE_DIR}/apps/EKiosk/src/DatabaseUtils/Database.qrc
    FOLDER "tests/apps/EKiosk"
    QT_MODULES Test Core
)

add_subdirectory(Example)
