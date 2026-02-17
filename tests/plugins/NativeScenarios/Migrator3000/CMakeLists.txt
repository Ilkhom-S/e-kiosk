ek_add_test(migrator3000_scenario_test
    FOLDER "tests/plugins"
    SOURCES migrator3000_scenario_test.cpp
    DEPENDS PluginTestCommon
    QT_MODULES Test Core
)
