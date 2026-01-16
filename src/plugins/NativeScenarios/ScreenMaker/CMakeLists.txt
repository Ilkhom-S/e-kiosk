# ScreenMaker scenario plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB SCREENMAKER_SOURCES
    src/*.cpp
    src/*.h
)

file(GLOB SCREENMAKER_RESOURCES
    src/*.qrc
)

# Handle Qt5/Qt6 declarative differences
if(QT_VERSION_MAJOR EQUAL 5)
    set(SM_QT_MODULES Gui Network Concurrent Quick)
else()
    set(SM_QT_MODULES Gui Network Concurrent Quick Qml)
endif()

ek_add_plugin(screen_maker_scenario
    FOLDER "plugins/NativeScenarios/ScreenMaker"
    SOURCES ${SCREENMAKER_SOURCES} ${SCREENMAKER_RESOURCES}
    QT_MODULES ${SM_QT_MODULES}
    DEPENDS
    App
    Log
    GUISDK
    NetworkTaskManager
    ScenarioEngine
)
