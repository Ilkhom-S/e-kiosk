# Tests for apps/EKiosk
message(STATUS "Configuring EKiosk app tests")

if(WIN32)
    add_executable(TestPrinterConversion TestPrinterConversion.cpp)
    set_target_properties(TestPrinterConversion PROPERTIES FOLDER "tests/apps/EKiosk")

    add_executable(TestStringConversions TestStringConversions.cpp)
    set_target_properties(TestStringConversions PROPERTIES FOLDER "tests/apps/EKiosk")

    add_executable(TestConnectPrivilege TestConnectPrivilege.cpp)
    set_target_properties(TestConnectPrivilege PROPERTIES FOLDER "tests/apps/EKiosk")

    add_executable(TestBaseApplication TestBaseApplication.cpp)
    set_target_properties(TestBaseApplication PROPERTIES FOLDER "tests/apps/EKiosk")
endif()

add_subdirectory(Example)
