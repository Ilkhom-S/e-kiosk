# ScenarioEngine module tests

file(GLOB SCENARIOENGINE_TEST_SOURCES
    *.cpp
    *.h
)

ek_add_test(ScenarioEngineTest
    FOLDER "tests/modules/ScenarioEngine"
    SOURCES ${SCENARIOENGINE_TEST_SOURCES}
    QT_MODULES Test Core Qml
    DEPENDS ScenarioEngine
)
