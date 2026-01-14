find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Test REQUIRED)
include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

ek_add_test(TestTemplate
    SOURCES TestTemplate.cpp
    QT_MODULES Test Core
)
