# WebKitBackend graphic plugin - Qt 5.0-5.5 only (deprecated)

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

# Only build WebKitBackend for Qt 5.0-5.5
if(NOT (QT_VERSION_MAJOR EQUAL 5 AND QT_VERSION_MINOR LESS 6))
    message(STATUS "WebKitBackend only supports Qt 5.0-5.5, skipping for Qt ${QT_VERSION}")
    return()
endif()

set(WEBKITBACKEND_SOURCES
    src/WebKitBackendFactory.cpp
    src/WebKitBackendFactory.h
    src/WebKitBackend.cpp
    src/WebKitBackend.h
    src/WebGraphicsItem.cpp
    src/WebGraphicsItem.h
    src/WebPageLogger.cpp
    src/WebPageLogger.h
)

# WebKit is only available in Qt 5.0-5.5 (deprecated in 5.5, removed in 5.6)
# This plugin is restricted to Qt 5.0-5.5 only
find_package(Qt5 COMPONENTS WebKit WebKitWidgets QUIET)
if(Qt5WebKit_FOUND)
    set(WEBKIT_QT_MODULES Widgets WebKit WebKitWidgets)
else()
    message(WARNING "WebKit not found for WebKitBackend - this plugin requires Qt 5.0-5.5")
    set(WEBKIT_QT_MODULES Widgets)
endif()

ek_add_plugin(webkit_backend
    FOLDER "plugins/GraphicBackends"
    SOURCES ${WEBKITBACKEND_SOURCES}
    QT_MODULES ${WEBKIT_QT_MODULES}
    DEPENDS PluginsSDK
    INSTALL_DIR "plugins/backends"
)
