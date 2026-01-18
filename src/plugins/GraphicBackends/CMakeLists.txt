# GraphicBackends plugins

add_subdirectory(NativeBackend)
add_subdirectory(QMLBackend)

# Add web backends based on Qt version
# WebKitBackend: Qt 5.0-5.5 only (deprecated)
# WebEngineBackend: Qt 5.6+ and Qt 6
if(QT_VERSION_MAJOR EQUAL 5 AND QT_VERSION_MINOR LESS 6)
    add_subdirectory(WebKitBackend)
else()
    add_subdirectory(WebEngineBackend)
endif()
