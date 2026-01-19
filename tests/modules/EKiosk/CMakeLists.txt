# Tests for apps/EKiosk module
message(STATUS "Configuring EKiosk module tests")

add_executable(TestBaseApplication TestBaseApplication.cpp)
set_target_properties(TestBaseApplication PROPERTIES FOLDER "tests/EKiosk")

add_executable(TestPrinterConversion TestPrinterConversion.cpp)
set_target_properties(TestPrinterConversion PROPERTIES FOLDER "tests/EKiosk")

add_executable(TestStringConversions TestStringConversions.cpp)
set_target_properties(TestStringConversions PROPERTIES FOLDER "tests/EKiosk")

add_executable(TestConnectPrivilege TestConnectPrivilege.cpp)
set_target_properties(TestConnectPrivilege PROPERTIES FOLDER "tests/EKiosk")

add_subdirectory(Example)
