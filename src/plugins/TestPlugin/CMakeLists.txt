# TestPlugin - minimal plugin for testing the plugin system

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB TESTPLUGIN_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(test_plugin
    FOLDER "plugins/Test"
    SOURCES ${TESTPLUGIN_SOURCES}
    QT_MODULES Core
    DEPENDS PluginsSDK
)
