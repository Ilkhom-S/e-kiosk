# ScenarioEngine module

file(GLOB SCENARIOENGINE_SOURCES
    src/*.cpp
    src/*.h
)

ek_add_library(ScenarioEngine
    SOURCES ${SCENARIOENGINE_SOURCES}
    QT_MODULES Core Concurrent
)
