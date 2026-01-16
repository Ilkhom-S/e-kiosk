# Migrator3000 scenario plugin

include(${CMAKE_SOURCE_DIR}/cmake/EKPlugin.cmake)

file(GLOB MIGRATOR_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_plugin(migrator3000_scenario
    FOLDER "plugins/NativeScenarios/Migrator3000"
    SOURCES ${MIGRATOR_SOURCES}
    QT_MODULES Core Network
    DEPENDS
    Log
    NetworkTaskManager
    ScenarioEngine
    ek_boost
)
