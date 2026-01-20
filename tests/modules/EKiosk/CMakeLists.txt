# Tests for apps/EKiosk module
message(STATUS "Configuring EKiosk module tests")

# TestBaseApplication is defined in tests/unit to avoid duplicate targets.
# If a module-specific build is required, rename the target here to avoid conflicts.

add_executable(TestPrinterConversion TestPrinterConversion.cpp)
set_target_properties(TestPrinterConversion PROPERTIES FOLDER "tests/EKiosk")

add_executable(TestStringConversions TestStringConversions.cpp)
set_target_properties(TestStringConversions PROPERTIES FOLDER "tests/EKiosk")

add_executable(TestConnectPrivilege TestConnectPrivilege.cpp)
set_target_properties(TestConnectPrivilege PROPERTIES FOLDER "tests/EKiosk")

add_subdirectory(Example)
