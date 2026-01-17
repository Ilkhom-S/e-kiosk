ek_add_test(ad_plugin_test
    FOLDER "tests/plugins"
    SOURCES ad_plugin_test.cpp
    DEPENDS PluginTestCommon ad
    QT_MODULES Test Core
)
