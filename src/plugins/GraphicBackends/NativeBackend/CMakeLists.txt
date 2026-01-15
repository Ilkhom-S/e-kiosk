# NativeBackend graphic plugin

file(GLOB NATIVEBACKEND_SOURCES
    src/*.cpp
    src/*.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_graphic_backend(native_backend
    SOURCES ${NATIVEBACKEND_SOURCES} ${VERSION_RC}
    QT_MODULES Core Widgets
    DEPENDS PluginsSDK
)

# Note: Depends on boost from thirdparty
