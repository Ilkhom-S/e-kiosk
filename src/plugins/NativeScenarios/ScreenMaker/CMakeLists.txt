# ScreenMaker scenario plugin

file(GLOB SCREENMAKER_SOURCES
    src/*.cpp
    src/*.h
)

file(GLOB SCREENMAKER_RESOURCES
    src/*.qrc
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

# Handle Qt5/Qt6 declarative differences
if(QT_VERSION_MAJOR EQUAL 5)
    set(SM_QT_MODULES Gui Network Concurrent Quick)
else()
    set(SM_QT_MODULES Gui Network Concurrent Quick Qml)
endif()

tc_add_scenario_backend(screen_maker_scenario
    SOURCES ${SCREENMAKER_SOURCES} ${SCREENMAKER_RESOURCES} ${VERSION_RC}
    QT_MODULES ${SM_QT_MODULES}
    DEPENDS
        App
        Log
        GUISDK
        NetworkTaskManager
        ScenarioEngine
)
