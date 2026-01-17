# Plugin testing common framework

set(PLUGIN_TEST_COMMON_SOURCES
    PluginTestBase.cpp
    PluginTestBase.h
)

# Create a static library for common testing utilities
add_library(PluginTestCommon STATIC ${PLUGIN_TEST_COMMON_SOURCES})

target_include_directories(PluginTestCommon
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(PluginTestCommon
    PUBLIC Qt${QT_VERSION_MAJOR}::Core
    PUBLIC Qt${QT_VERSION_MAJOR}::Test
    PUBLIC PluginsSDK
)
