# QMLBackend graphic plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB QMLBACKEND_SOURCES
    src/*.cpp
    src/*.h
)

# Handle Qt5/Qt6 differences for declarative/quick modules
if(QT_VERSION_MAJOR EQUAL 5)
    set(QML_QT_MODULES Widgets Quick)
    find_package(Qt5 COMPONENTS Script QUIET)
    if(Qt5Script_FOUND)
        list(APPEND QML_QT_MODULES Script)
    endif()
else()
    set(QML_QT_MODULES Widgets Quick Qml)
endif()

ek_add_plugin(qml_backend
    SOURCES ${QMLBACKEND_SOURCES}
    QT_MODULES ${QML_QT_MODULES}
    DEPENDS PluginsSDK
)
