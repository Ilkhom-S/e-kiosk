# ScreenMaker scenario plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

set(SCREENMAKER_SOURCES
    src/ScreenMakerFactory.cpp
    src/ScreenMakerFactory.h
    src/MainScenario.cpp
    src/MainScenario.h
    src/ScenarioPlugin.h
)

ek_add_plugin(screen_maker_scenario
    FOLDER "plugins/NativeScenarios"
    SOURCES ${SCREENMAKER_SOURCES}
    QT_MODULES Gui Network Concurrent
    DEPENDS
    Log
    ScenarioEngine
    PluginsSDK
    PPSDK
    NetworkTaskManager
)
