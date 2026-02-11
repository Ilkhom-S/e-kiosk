# Deprecated Code - Build Configuration
# This CMakeLists.txt is NOT included by default
# Enable with: cmake -DBUILD_DEPRECATED=ON

if(BUILD_DEPRECATED)
    add_subdirectory(ekiosk-src)
    message(STATUS "Building deprecated code (for reference only)")
else()
    message(STATUS "Skipping deprecated code (use -DBUILD_DEPRECATED=ON to include)")
endif()
