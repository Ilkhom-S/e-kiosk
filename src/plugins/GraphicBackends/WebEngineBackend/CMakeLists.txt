# WebEngineBackend graphic plugin - Qt 6 only

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

# Only build WebEngineBackend for Qt 6
if(NOT QT_VERSION_MAJOR EQUAL 6)
    return()
endif()

set(WEBENGINEBACKEND_SOURCES
    src/WebEngineBackendFactory.cpp
    src/WebEngineBackendFactory.h
    src/WebEngineBackend.cpp
    src/WebEngineBackend.h
    src/WebKitBackend.cpp
    src/WebKitBackend.h
    src/WebGraphicsItem.cpp
    src/WebGraphicsItem.h
    src/WebPageLogger.cpp
    src/WebPageLogger.h
    src/webengine_backend.json
)

# Qt6 only has WebEngine
find_package(Qt6 COMPONENTS WebEngineWidgets QUIET)
if(Qt6WebEngineWidgets_FOUND)
    set(WEBENGINE_QT_MODULES Widgets WebEngineWidgets)
    add_compile_definitions(USE_WEBENGINE)
else()
    message(WARNING "WebEngine not found for WebEngineBackend in Qt6")
    set(WEBENGINE_QT_MODULES Widgets)
endif()

ek_add_plugin(webengine_backend
    FOLDER "plugins/GraphicBackends/WebEngineBackend"
    SOURCES ${WEBENGINEBACKEND_SOURCES}
    QT_MODULES ${WEBENGINE_QT_MODULES}
    DEPENDS PluginsSDK
)
