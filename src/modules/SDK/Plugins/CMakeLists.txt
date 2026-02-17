# PluginsSDK module

file(GLOB PLUGINSSDK_SOURCES
    src/*.cpp
    src/*.h
)

# Include the public header for moc generation
list(APPEND PLUGINSSDK_SOURCES ${EK_INCLUDES_DIR}/SDK/Plugins/PluginFactory.h)

ek_add_library(PluginsSDK
    FOLDER "modules/SDK"
    SOURCES ${PLUGINSSDK_SOURCES}
    QT_MODULES Core
    DEPENDS DriversSDK PPSDK
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Prevent the static library from embedding Q_PLUGIN_METADATA symbols. Plugins (dlls) that
# link with this library will include the metadata via headers during their own build.
target_compile_definitions(PluginsSDK PRIVATE SDK_PLUGINS_NO_METADATA)

# Set minimum Windows version
if(WIN32)
    target_compile_definitions(PluginsSDK PRIVATE _WIN32_WINNT=0x0502)
endif()
