# EKiosk Tests CMake

# Add subdirectories for each app/module's tests here
add_subdirectory(unit)

# Enable testing
include(CTest)
enable_testing()
