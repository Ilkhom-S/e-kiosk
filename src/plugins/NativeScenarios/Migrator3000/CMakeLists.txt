# Migrator3000 scenario plugin

file(GLOB MIGRATOR_SOURCES
    src/*.cpp
    src/*.h
)

set(VERSION_RC ${TC_INCLUDES_DIR}/Common/CoreVersion.rc)

tc_add_scenario_backend(migrator3000_scenario
    SOURCES ${MIGRATOR_SOURCES} ${VERSION_RC}
    QT_MODULES Core Network
    DEPENDS
        Log
        NetworkTaskManager
        ScenarioEngine
)
