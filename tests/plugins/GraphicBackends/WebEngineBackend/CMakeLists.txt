ek_add_test(webengine_backend_test
    FOLDER "tests/plugins"
    SOURCES webengine_backend_test.cpp
    DEPENDS PluginTestCommon webengine_backend
    QT_MODULES Test Core Widgets
)

# Copy the plugin DLL to the test directory for testing
add_custom_command(TARGET webengine_backend_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:webengine_backend>
        $<TARGET_FILE_DIR:webengine_backend_test>
    COMMENT "Copying webengine_backend plugin to test directory"
)