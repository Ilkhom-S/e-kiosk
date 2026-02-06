# NativeBackend graphic plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

set(NATIVEBACKEND_SOURCES
    src/NativeBackendFactory.cpp
    src/NativeBackendFactory.h
    src/NativeBackend.cpp
    src/NativeBackend.h
)

set(VERSION_RC ${EK_INCLUDES_DIR}/Common/CoreVersion.rc)

ek_add_plugin(native_backend
    FOLDER "plugins/GraphicBackends"
    SOURCES ${NATIVEBACKEND_SOURCES}
    QT_MODULES Core Widgets
    DEPENDS PluginsSDK ek_common ek_boost
    INSTALL_DIR "plugins/backends"
)

# Note: Depends on boost from thirdparty
