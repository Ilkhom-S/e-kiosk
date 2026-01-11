# Unit Tests for EKiosk

# Example: add_subdirectory(kiosk)

# Add example test
file(GLOB EXAMPLE_SOURCES Example/*.cpp Example/*.h)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Example")
    add_executable(TestExample ${EXAMPLE_SOURCES})
    find_package(Qt5 COMPONENTS Test REQUIRED)
    target_link_libraries(TestExample PRIVATE Qt5::Test)
    add_test(NAME TestExample COMMAND TestExample)
endif()
