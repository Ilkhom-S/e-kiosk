# WebEngineBackend graphic plugin - Qt 5.6+ and Qt 6

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

# Only build WebEngineBackend for Qt 5.6+ and Qt 6
if(QT_VERSION_MAJOR EQUAL 5 AND QT_VERSION_MINOR LESS 6)
    message(STATUS "WebEngineBackend requires Qt 5.6+ or Qt 6, skipping for Qt ${QT_VERSION}")
    return()
endif()

set(WEBENGINEBACKEND_SOURCES
    src/WebEngineBackendFactory.cpp
    src/WebEngineBackendFactory.h
    src/WebEngineBackend.cpp
    src/WebEngineBackend.h
    src/WebGraphicsItem.cpp
    src/WebGraphicsItem.h
    src/WebPageLogger.cpp
    src/WebPageLogger.h
)

# Find WebEngine components (version-agnostic)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS WebEngineWidgets QUIET)
if(Qt${QT_VERSION_MAJOR}WebEngineWidgets_FOUND)
    set(WEBENGINE_QT_MODULES Widgets WebEngineWidgets)
    add_compile_definitions(USE_WEBENGINE)
else()
    message(WARNING "WebEngine not found for WebEngineBackend in Qt${QT_VERSION_MAJOR}")
    set(WEBENGINE_QT_MODULES Widgets)
endif()

ek_add_plugin(webengine_backend
    FOLDER "plugins/GraphicBackends"
    SOURCES ${WEBENGINEBACKEND_SOURCES}
    QT_MODULES ${WEBENGINE_QT_MODULES}
    DEPENDS PluginsSDK
)
