# TemplatePlugin unit tests

include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

include_directories(${CMAKE_SOURCE_DIR}/src/plugins/TemplatePlugin/src)
include_directories(${CMAKE_SOURCE_DIR}/include)

ek_add_test(template_plugin_test
    FOLDER "tests/plugins"
    SOURCES test_plugin_test.cpp
    DEPENDS template_plugin
    QT_MODULES Test Core
)
