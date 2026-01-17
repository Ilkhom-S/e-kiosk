# TestPlugin unit tests

include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

include_directories(${CMAKE_SOURCE_DIR}/src/plugins/TestPlugin/src)
include_directories(${CMAKE_SOURCE_DIR}/include)

ek_add_test(test_plugin_test
    FOLDER "tests/plugins"
    SOURCES test_plugin_test.cpp
    DEPENDS test_plugin
    QT_MODULES Test Core
)
