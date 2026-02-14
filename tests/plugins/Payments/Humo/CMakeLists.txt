ek_add_test(humo_payments_plugin_test
    FOLDER "tests/plugins"
    SOURCES humo_payments_plugin_test.cpp
    DEPENDS PluginTestCommon humo_payments
    QT_MODULES Test Core
)
