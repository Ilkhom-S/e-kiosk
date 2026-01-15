# WebKitBackend graphic plugin - Qt 5 only

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

# Only build WebKitBackend for Qt 5
if(NOT QT_VERSION_MAJOR EQUAL 5)
    return()
endif()

file(GLOB WEBKITBACKEND_SOURCES
    src/*.cpp
    src/*.h
)

# WebKit is deprecated in Qt5.5+ and removed in Qt6
# Use WebEngine as alternative for Qt5.6+
find_package(Qt5 COMPONENTS WebKit WebKitWidgets QUIET)
if(Qt5WebKit_FOUND)
    set(WEBKIT_QT_MODULES Widgets WebKit WebKitWidgets)
else()
    # Try WebEngine as fallback
    find_package(Qt5 COMPONENTS WebEngine WebEngineWidgets QUIET)
    if(Qt5WebEngine_FOUND)
        set(WEBKIT_QT_MODULES Widgets WebEngine WebEngineWidgets)
        add_compile_definitions(USE_WEBENGINE)
    else()
        message(WARNING "Neither WebKit nor WebEngine found for WebKitBackend")
        set(WEBKIT_QT_MODULES Widgets)
    endif()
endif()

ek_add_plugin(webkit_backend
    SOURCES ${WEBKITBACKEND_SOURCES}
    QT_MODULES ${WEBKIT_QT_MODULES}
    DEPENDS PluginsSDK
)
