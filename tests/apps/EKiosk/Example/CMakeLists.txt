# Example test for EKiosk app
if(WIN32)
    add_executable(TestExample TestExample.cpp)
    set_target_properties(TestExample PROPERTIES FOLDER "tests/apps/EKiosk/Example")
endif()
