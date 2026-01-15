# QMLBackend graphic plugin

file(GLOB QMLBACKEND_SOURCES
    src/*.cpp
    src/*.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

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

tc_add_graphic_backend(qml_backend
    SOURCES ${QMLBACKEND_SOURCES} ${VERSION_RC}
    QT_MODULES ${QML_QT_MODULES}
    DEPENDS PluginsSDK
)
