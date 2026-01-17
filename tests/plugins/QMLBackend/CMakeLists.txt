ek_add_test(qml_backend_test
    FOLDER "tests/plugins"
    SOURCES qml_backend_test.cpp
    DEPENDS PluginTestCommon PluginsSDK
    QT_MODULES Test Core
)
