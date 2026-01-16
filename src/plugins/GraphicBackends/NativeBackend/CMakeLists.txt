# NativeBackend graphic plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB NATIVEBACKEND_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(native_backend
    FOLDER "plugins/GraphicBackends/NativeBackend"
    SOURCES ${NATIVEBACKEND_SOURCES}
    QT_MODULES Core Widgets
    DEPENDS PluginsSDK ek_common
)

# Note: Depends on boost from thirdparty
