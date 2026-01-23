# Interface CMakeLists.txt - Utils dynamic library

file(GLOB UTILS_SOURCES
    plugins/Utils/src/*.cpp
    plugins/Utils/src/*.h
)

set(UTILS_VERSION_RC ${EK_INCLUDES_DIR}/Common/CoreVersion.rc)

# Determine Qt modules - declarative is deprecated in Qt5.12+/Qt6
if(QT_VERSION_MAJOR EQUAL 5)
    set(UTILS_QT_MODULES Core Gui Widgets Xml Multimedia)
    find_package(Qt5 COMPONENTS Quick QUIET)
    if(Qt5Quick_FOUND)
        list(APPEND UTILS_QT_MODULES Quick)
    endif()
else()
    set(UTILS_QT_MODULES Core Gui Widgets Xml Multimedia Quick Qml)
endif()

ek_add_plugin(utils
    SOURCES ${UTILS_SOURCES} ${UTILS_VERSION_RC}
    QT_MODULES ${UTILS_QT_MODULES}
    INSTALL_DIR "plugins/interface"
    DEPENDS QZint
)
