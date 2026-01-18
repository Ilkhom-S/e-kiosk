include(${CMAKE_SOURCE_DIR}/cmake/EKTesting.cmake)

set(SCREENMAKER_TEST_SOURCES
    screen_maker_scenario_test.cpp
)

ek_add_test(screen_maker_scenario_test
    FOLDER "tests/plugins/NativeScenarios"
    SOURCES ${SCREENMAKER_TEST_SOURCES}
    DEPENDS PluginTestCommon screen_maker_scenario
    QT_MODULES Test Core
)
