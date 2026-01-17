ek_add_test(native_backend_test
    FOLDER "tests/plugins"
    SOURCES native_backend_test.cpp
    DEPENDS PluginTestCommon native_backend
    QT_MODULES Test Core Widgets
)

# Copy the plugin DLL to the test directory for testing
add_custom_command(TARGET native_backend_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:native_backend>
    $<TARGET_FILE_DIR:native_backend_test>
    COMMENT "Copying native_backend plugin to test directory"
)
