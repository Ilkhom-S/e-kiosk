# GraphicBackends plugins

add_subdirectory(NativeBackend)
add_subdirectory(QMLBackend)

# Add web backends based on Qt version
if(QT_VERSION_MAJOR EQUAL 5)
    add_subdirectory(WebKitBackend)
else()
    add_subdirectory(WebEngineBackend)
endif()
