
# Unit Tests for EKiosk


find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

file(GLOB EXAMPLE_SOURCES Example/*.cpp Example/*.h)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Example")
    ek_add_test(TestExample
        SOURCES ${EXAMPLE_SOURCES}
        QT_MODULES Test
    )
endif()
