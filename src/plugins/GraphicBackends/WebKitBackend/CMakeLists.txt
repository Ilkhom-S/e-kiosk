# WebKitBackend graphic plugin

file(GLOB WEBKITBACKEND_SOURCES
    src/*.cpp
    src/*.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

# WebKit is deprecated in Qt5.5+ and removed in Qt6
# Use WebEngine as alternative for Qt5.6+/Qt6
if(QT_VERSION_MAJOR EQUAL 5)
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
else()
    # Qt6 only has WebEngine
    find_package(Qt6 COMPONENTS WebEngineWidgets QUIET)
    if(Qt6WebEngineWidgets_FOUND)
        set(WEBKIT_QT_MODULES Widgets WebEngineWidgets)
        add_compile_definitions(USE_WEBENGINE)
    else()
        message(WARNING "WebEngine not found for WebKitBackend in Qt6")
        set(WEBKIT_QT_MODULES Widgets)
    endif()
endif()

tc_add_graphic_backend(webkit_backend
    SOURCES ${WEBKITBACKEND_SOURCES} ${VERSION_RC}
    QT_MODULES ${WEBKIT_QT_MODULES}
    DEPENDS PluginsSDK
)
