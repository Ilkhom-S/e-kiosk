# PluginsSDK module

file(GLOB PLUGINSSDK_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_library(PluginsSDK
    SOURCES ${PLUGINSSDK_SOURCES}
    QT_MODULES Core
)

# Set minimum Windows version
if(WIN32)
    target_compile_definitions(PluginsSDK PRIVATE _WIN32_WINNT=0x0502)
endif()
