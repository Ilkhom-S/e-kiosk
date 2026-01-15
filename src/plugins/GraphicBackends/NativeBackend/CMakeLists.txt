# NativeBackend graphic plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB NATIVEBACKEND_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(native_backend
    SOURCES ${NATIVEBACKEND_SOURCES}
    QT_MODULES Core Widgets
    DEPENDS PluginsSDK
)

# Note: Depends on boost from thirdparty
