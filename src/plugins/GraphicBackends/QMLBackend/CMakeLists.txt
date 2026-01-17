# QMLBackend graphic plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

set(QMLBACKEND_SOURCES
    src/QMLBackendFactory.cpp
    src/QMLBackendFactory.h
    src/QMLBackend.cpp
    src/QMLBackend.h
    src/QMLGraphicsItem.cpp
    src/QMLGraphicsItem.h
    src/Md5ValidatorQmlItem.h
)

# Handle Qt5/Qt6 differences for declarative/quick modules
if(QT_VERSION_MAJOR EQUAL 5)
    find_package(Qt5 COMPONENTS Widgets Quick WebEngine REQUIRED)
    set(QML_QT_MODULES Widgets Quick WebEngine)
    find_package(Qt5 COMPONENTS Script QUIET)
    if(Qt5Script_FOUND)
        list(APPEND QML_QT_MODULES Script)
    endif()
else()
    find_package(Qt6 COMPONENTS Widgets Quick Qml WebEngineCore REQUIRED)
    set(QML_QT_MODULES Widgets Quick Qml WebEngineCore)
endif()

ek_add_plugin(qml_backend
    FOLDER "plugins/GraphicBackends"
    SOURCES ${QMLBACKEND_SOURCES}
    QT_MODULES ${QML_QT_MODULES}
    DEPENDS PluginsSDK ek_boost PPSDK GUISDK
)
