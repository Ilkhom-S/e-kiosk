# Plugin testing common framework

include(${CMAKE_SOURCE_DIR}/cmake/EKLibrary.cmake)

set(PLUGIN_TEST_COMMON_SOURCES
    PluginTestBase.cpp
    PluginTestBase.h
)

# Create a static library for common testing utilities
ek_add_library(PluginTestCommon
    FOLDER "tests/plugins/common"
    SOURCES ${PLUGIN_TEST_COMMON_SOURCES}
    QT_MODULES Core Test
    DEPENDS PluginsSDK PPSDK
)
